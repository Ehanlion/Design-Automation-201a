#!/usr/bin/env bash
set -euo pipefail

# @purpose: Execute Lab 4 Part 2 experiments in both placement modes.
# @logic: Run one timing-driven and one non-timing-driven Innovus flow.
# why: Problem 2 requires direct metric comparison between these two runs.
#
# Usage:
#   ./scripts/runLab4Part2.sh [target_utilization]
# Example:
#   ./scripts/runLab4Part2.sh 0.991

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PART2_TCL="${LAB4_DIR}/lab4_part2.tcl"

UTIL="${1:-0.991}"
if ! awk "BEGIN { exit !(${UTIL} > 0.0 && ${UTIL} < 1.0) }"; then
    echo "Error: utilization must be strictly between 0 and 1."
    exit 1
fi

if [ ! -f "$PART2_TCL" ]; then
    echo "Error: Tcl script not found: $PART2_TCL"
    exit 1
fi

RUN_ID="$(date +%Y%m%d_%H%M%S)_$$"
RUN_DIR="${LAB4_DIR}/output/part2_run_${RUN_ID}_util${UTIL}"
TD_DIR="${RUN_DIR}/timing_driven"
NTD_DIR="${RUN_DIR}/no_timing_driven"
SUMMARY_CSV="${RUN_DIR}/part2_metrics.csv"
SUMMARY_MD="${RUN_DIR}/part2_metrics.md"
SUMMARY_TXT="${RUN_DIR}/part2_final_comparison.txt"
LATEST_RUN_PTR="${LAB4_DIR}/output/part2_latest_run.txt"

mkdir -p "$TD_DIR" "$NTD_DIR"

# @purpose: Source CAD environment only if needed.
# @logic: Reuse the same setup sequence as existing lab4 run scripts.
# why: Keeps Part 2 runnable on a fresh shell session.
ensure_innovus_in_path() {
    if command -v innovus >/dev/null 2>&1; then
        return
    fi

    echo "innovus not in PATH — sourcing Cadence/Mentor SETUP files ..."
    set +eu
    source /w/apps3/Cadence/INNOVUSEXPORT21/SETUP
    source /w/apps3/Cadence/GENUS211/SETUP
    source /w/apps3/Mentor/Calibre/2024.3_16.10/SETUP

    export PATH="/w/apps3/Cadence/CONFRML241/bin:${PATH}"
    export PATH="/w/class.1/ee/ee201a/ee201ata/oa/bin/linux_rhel40_64/opt:${PATH}"
    export OA_PLUGIN_PATH=/w/class.1/ee/ee201a/ee201ata/oa/data/plugins
    export LD_LIBRARY_PATH="/w/class.1/ee/ee201a/ee201ata/oa/lib/linux_rhel40_64/opt:${LD_LIBRARY_PATH:-}"
    export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/usr/lib64:/usr/lib:/lib64:/lib"
    export COMPILER_PATH=/usr
    set -eu
}

# @purpose: Run a single Part 2 experiment mode.
# @logic: Pass mode/output settings through environment variables.
# why: One Tcl script handles both experiment variants.
run_one_mode() {
    local mode_label="$1"
    local timing_driven="$2"
    local out_dir="$3"
    local console_log="${out_dir}/innovus_console.log"
    local exit_code=0

    echo ""
    echo "=========================================="
    echo "  Lab 4 Part 2 — ${mode_label}"
    echo "=========================================="
    echo "  Timing driven: ${timing_driven}"
    echo "  Utilization:   ${UTIL}"
    echo "  Output dir:    ${out_dir}"
    echo "  Console log:   ${console_log}"
    echo "=========================================="

    (
        cd "$LAB4_DIR"
        LAB4_OUTPUTDIR="$out_dir" \
        LAB4_UTIL="$UTIL" \
        LAB4_TIMING_DRIVEN="$timing_driven" \
        innovus -batch -no_gui -init "$PART2_TCL"
    ) 2>&1 | tee "$console_log" || exit_code=$?

    echo "Mode '${mode_label}' finished with exit code: ${exit_code}"
    return "$exit_code"
}

# @purpose: Parse worst slack from a timing report.
# @logic: Read the first 'Slack Time' value written by Innovus.
# why: Needed to compare timing quality across checkpoints.
extract_slack() {
    local rpt="$1"
    if [ ! -f "$rpt" ]; then
        echo "N/A"
        return
    fi
    local value
    value="$(grep -m1 'Slack Time' "$rpt" 2>/dev/null | awk '{print $NF}' || true)"
    if [ -z "$value" ]; then
        echo "N/A"
    else
        echo "$value"
    fi
}

# @purpose: Parse total power from a power report.
# @logic: Prefer 'Total Power' line; fallback to first total-*power line.
# why: Keeps extraction robust if report formatting changes slightly.
extract_power() {
    local rpt="$1"
    if [ ! -f "$rpt" ]; then
        echo "N/A N/A"
        return
    fi

    local line
    line="$(awk '/Total Power:[[:space:]]*[-+]?[0-9]/ {print; exit}' "$rpt")"
    if [ -z "$line" ]; then
        echo "N/A N/A"
        return
    fi

    local value unit
    value="$(echo "$line" | awk -F: '{gsub(/^[ \t]+|[ \t]+$/, "", $2); split($2, a, /[ \t]+/); print a[1]}')"
    unit="$(awk -F= '/Power Units/ {gsub(/[[:space:]]/, "", $2); print $2; exit}' "$rpt")"
    if [ -z "$unit" ]; then
        unit="mW"
    fi

    if [ -z "$value" ]; then
        echo "N/A N/A"
    else
        echo "$value $unit"
    fi
}

# @purpose: Generate CSV/Markdown checkpoint comparison.
# @logic: Parse matching reports from both run directories.
# why: Produces reproducible artifacts for documentation and validation.
build_comparison_reports() {
    local -a checkpoints=(
        "before_placement"
        "after_placement"
        "after_opt_prects"
        "after_extractrc_postcts"
        "after_opt_postcts_hold"
        "after_extractrc_preroute"
        "after_global_route"
        "after_detail_route"
        "after_opt_postroute_setup"
        "after_opt_postroute_hold"
        "after_extractrc_postroute"
        "final_postroute"
    )

    echo "run_mode,checkpoint,setup_slack_ns,hold_slack_ns,total_power_value,total_power_unit" > "$SUMMARY_CSV"

    local mode base cp setup hold pval punit
    for mode in timing_driven no_timing_driven; do
        if [ "$mode" = "timing_driven" ]; then
            base="$TD_DIR"
        else
            base="$NTD_DIR"
        fi

        for cp in "${checkpoints[@]}"; do
            setup="$(extract_slack "${base}/metrics/${mode}_${cp}_setup.tarpt")"
            hold="$(extract_slack "${base}/metrics/${mode}_${cp}_hold.tarpt")"
            read -r pval punit <<< "$(extract_power "${base}/metrics/${mode}_${cp}_power.rpt")"
            echo "${mode},${cp},${setup},${hold},${pval},${punit}" >> "$SUMMARY_CSV"
        done
    done

    {
        echo "# Lab 4 Part 2 Metrics"
        echo ""
        echo "Run directory: ${RUN_DIR}"
        echo ""
        echo "| Run Mode | Checkpoint | Setup Slack (ns) | Hold Slack (ns) | Total Power |"
        echo "|---|---|---:|---:|---:|"
        tail -n +2 "$SUMMARY_CSV" | while IFS=, read -r run_mode checkpoint setup_s hold_s power_v power_u; do
            echo "| ${run_mode} | ${checkpoint} | ${setup_s} | ${hold_s} | ${power_v} ${power_u} |"
        done
    } > "$SUMMARY_MD"

    local td_setup td_hold td_power td_unit ntd_setup ntd_hold ntd_power ntd_unit
    td_setup="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $3}' "$SUMMARY_CSV")"
    td_hold="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $4}' "$SUMMARY_CSV")"
    td_power="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $5}' "$SUMMARY_CSV")"
    td_unit="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $6}' "$SUMMARY_CSV")"

    ntd_setup="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $3}' "$SUMMARY_CSV")"
    ntd_hold="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $4}' "$SUMMARY_CSV")"
    ntd_power="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $5}' "$SUMMARY_CSV")"
    ntd_unit="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $6}' "$SUMMARY_CSV")"

    {
        echo "Lab 4 Part 2 Final Checkpoint Comparison"
        echo "Run directory: ${RUN_DIR}"
        echo ""
        echo "timing_driven:"
        echo "  setup_slack_ns=${td_setup}"
        echo "  hold_slack_ns=${td_hold}"
        echo "  total_power=${td_power} ${td_unit}"
        echo ""
        echo "no_timing_driven:"
        echo "  setup_slack_ns=${ntd_setup}"
        echo "  hold_slack_ns=${ntd_hold}"
        echo "  total_power=${ntd_power} ${ntd_unit}"
    } > "$SUMMARY_TXT"
}

echo "=========================================="
echo "  Lab 4 Part 2 Runner"
echo "=========================================="
echo "  Tcl script:   $PART2_TCL"
echo "  Utilization:  $UTIL"
echo "  Run id:       $RUN_ID"
echo "  Run dir:      $RUN_DIR"
echo "=========================================="

ensure_innovus_in_path
if ! command -v innovus >/dev/null 2>&1; then
    echo "Error: innovus not found after sourcing SETUP files."
    exit 1
fi

echo "Using innovus: $(which innovus)"

set +e
run_one_mode "timing-driven placement" "true" "$TD_DIR"
TD_EXIT=$?
run_one_mode "non-timing-driven placement" "false" "$NTD_DIR"
NTD_EXIT=$?
set -e

if [ "$TD_EXIT" -ne 0 ] || [ "$NTD_EXIT" -ne 0 ]; then
    echo ""
    echo "ERROR: One or more Part 2 runs failed."
    echo "  timing-driven exit code:     $TD_EXIT"
    echo "  non-timing-driven exit code: $NTD_EXIT"
    echo "Check logs under: $RUN_DIR"
    exit 2
fi

build_comparison_reports

echo "$RUN_DIR" > "$LATEST_RUN_PTR"

echo ""
echo "=========================================="
echo "  Lab 4 Part 2 complete"
echo "=========================================="
echo "  timing-driven output:      $TD_DIR"
echo "  non-timing-driven output:  $NTD_DIR"
echo "  summary csv:               $SUMMARY_CSV"
echo "  summary markdown:          $SUMMARY_MD"
echo "  final comparison:          $SUMMARY_TXT"
echo "  latest run pointer:        $LATEST_RUN_PTR"
echo "=========================================="

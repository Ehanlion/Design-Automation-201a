#!/usr/bin/env bash
set -euo pipefail

# Collects submission-related artifacts into lab4/results.
# Safe to run multiple times.

PREFIX="Owen-Ethan_905452983_palatics"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
RESULTS_DIR="${LAB4_DIR}/results"
SUBMIT_DIR="${RESULTS_DIR}/submission_ready"
PART2_RUNS_DIR="${RESULTS_DIR}/part2_runs"
RUN_SCRIPTS_DIR="${RESULTS_DIR}/run_scripts"

mkdir -p "$RESULTS_DIR" "$SUBMIT_DIR" "$PART2_RUNS_DIR" "$RUN_SCRIPTS_DIR"

copy_file_if_exists() {
    local src="$1"
    local dst="$2"
    if [ -f "$src" ]; then
        cp "$src" "$dst"
        echo "COPY: $src -> $dst"
    else
        echo "MISS: $src"
    fi
}

copy_dir_if_exists() {
    local src="$1"
    local dst="$2"
    if [ -d "$src" ]; then
        rm -rf "$dst"
        cp -R "$src" "$dst"
        echo "COPY: $src -> $dst"
    else
        echo "MISS: $src"
    fi
}

latest_output_match() {
    local pattern="$1"
    if [ ! -d "$LAB4_DIR/output" ]; then
        return
    fi

    local matches=()
    local newest=""

    while IFS= read -r -d '' path; do
        matches+=("$path")
    done < <(find "$LAB4_DIR/output" -maxdepth 4 -name "$pattern" -print0 2>/dev/null)

    if [ "${#matches[@]}" -eq 0 ]; then
        return
    fi

    newest="$(ls -1t "${matches[@]}" 2>/dev/null | head -n 1 || true)"
    printf "%s" "$newest"
}

echo "=========================================="
echo "  Update lab4/results"
echo "=========================================="

# Stage run scripts for reproducibility.
copy_file_if_exists "$LAB4_DIR/scripts/runLab4Part1.sh" "$RUN_SCRIPTS_DIR/runLab4Part1.sh"
copy_file_if_exists "$LAB4_DIR/scripts/runLab4Part2TD.sh" "$RUN_SCRIPTS_DIR/runLab4Part2TD.sh"
copy_file_if_exists "$LAB4_DIR/scripts/runLab4Part2NoTD.sh" "$RUN_SCRIPTS_DIR/runLab4Part2NoTD.sh"
copy_file_if_exists "$LAB4_DIR/scripts/runLab4Part3.sh" "$RUN_SCRIPTS_DIR/runLab4Part3.sh"
copy_file_if_exists "$LAB4_DIR/scripts/clean.sh" "$RUN_SCRIPTS_DIR/clean.sh"
copy_file_if_exists "$LAB4_DIR/scripts/tar.sh" "$RUN_SCRIPTS_DIR/tar.sh"
copy_file_if_exists "$LAB4_DIR/scripts/submit.sh" "$RUN_SCRIPTS_DIR/submit.sh"

# Part 1 required submission artifacts.
copy_file_if_exists "$LAB4_DIR/lab4_part1.tcl" "$SUBMIT_DIR/${PREFIX}_Lab4_1.tcl"
P1_INVS="$LAB4_DIR/output/s1494_part1.invs"
P1_INVS_DAT="$LAB4_DIR/output/s1494_part1.invs.dat"
if [ ! -e "$P1_INVS" ]; then
    P1_INVS="$(latest_output_match "s1494_part1.invs")"
fi
if [ ! -e "$P1_INVS_DAT" ]; then
    P1_INVS_DAT="$(latest_output_match "s1494_part1.invs.dat")"
fi
if [ -n "${P1_INVS:-}" ] && [ -e "$P1_INVS" ]; then
    if [ -d "$P1_INVS" ]; then
        copy_dir_if_exists "$P1_INVS" "$SUBMIT_DIR/${PREFIX}_Lab4_1.invs"
    else
        copy_file_if_exists "$P1_INVS" "$SUBMIT_DIR/${PREFIX}_Lab4_1.invs"
    fi
else
    echo "MISS: Part 1 .invs"
fi
if [ -n "${P1_INVS_DAT:-}" ] && [ -e "$P1_INVS_DAT" ]; then
    copy_dir_if_exists "$P1_INVS_DAT" "$SUBMIT_DIR/${PREFIX}_Lab4_1.invs.dat"
else
    echo "MISS: Part 1 .invs.dat"
fi

# Part 2 scripts and run outputs.
copy_file_if_exists "$LAB4_DIR/lab4_part2.tcl" "$SUBMIT_DIR/${PREFIX}_Lab4_2.tcl"
copy_file_if_exists "$LAB4_DIR/lab4_part2_no_timing_driven.tcl" "$SUBMIT_DIR/${PREFIX}_Lab4_2_no_timing_driven.tcl"
copy_file_if_exists "$LAB4_DIR/lab4_part2_common.tcl" "$SUBMIT_DIR/${PREFIX}_Lab4_2_common.tcl"

TD_RUN_DIR=""
if [ -f "$LAB4_DIR/output/part2_td_latest_run.txt" ]; then
    TD_RUN_DIR="$(cat "$LAB4_DIR/output/part2_td_latest_run.txt")"
    copy_dir_if_exists "$TD_RUN_DIR" "$PART2_RUNS_DIR/timing_driven_latest"
    copy_file_if_exists "$TD_RUN_DIR/s1494_timing_driven_part2.invs" "$SUBMIT_DIR/${PREFIX}_Lab4_2_timing_driven.invs"
    copy_dir_if_exists "$TD_RUN_DIR/s1494_timing_driven_part2.invs.dat" "$SUBMIT_DIR/${PREFIX}_Lab4_2_timing_driven.invs.dat"
    copy_file_if_exists "$TD_RUN_DIR/timing_driven_summary.rpt" "$SUBMIT_DIR/${PREFIX}_Lab4_2_timing_driven_summary.rpt"
    copy_file_if_exists "$TD_RUN_DIR/innovus_console.log" "$SUBMIT_DIR/${PREFIX}_Lab4_2_timing_driven_console.log"
else
    echo "MISS: output/part2_td_latest_run.txt"
fi

NOTD_RUN_DIR=""
if [ -f "$LAB4_DIR/output/part2_notd_latest_run.txt" ]; then
    NOTD_RUN_DIR="$(cat "$LAB4_DIR/output/part2_notd_latest_run.txt")"
    copy_dir_if_exists "$NOTD_RUN_DIR" "$PART2_RUNS_DIR/no_timing_driven_latest"
    copy_file_if_exists "$NOTD_RUN_DIR/s1494_no_timing_driven_part2.invs" "$SUBMIT_DIR/${PREFIX}_Lab4_2_no_timing_driven.invs"
    copy_dir_if_exists "$NOTD_RUN_DIR/s1494_no_timing_driven_part2.invs.dat" "$SUBMIT_DIR/${PREFIX}_Lab4_2_no_timing_driven.invs.dat"
    copy_file_if_exists "$NOTD_RUN_DIR/no_timing_driven_summary.rpt" "$SUBMIT_DIR/${PREFIX}_Lab4_2_no_timing_driven_summary.rpt"
    copy_file_if_exists "$NOTD_RUN_DIR/innovus_console.log" "$SUBMIT_DIR/${PREFIX}_Lab4_2_no_timing_driven_console.log"
else
    echo "MISS: output/part2_notd_latest_run.txt"
fi

# Build latest Part 2 comparison files from TD and no-TD runs.
if [ -n "$TD_RUN_DIR" ] && [ -n "$NOTD_RUN_DIR" ] && [ -d "$TD_RUN_DIR" ] && [ -d "$NOTD_RUN_DIR" ]; then
    P2_CSV="$RESULTS_DIR/part2_metrics_combined.csv"
    P2_MD="$RESULTS_DIR/part2_metrics_combined.md"
    P2_TXT="$RESULTS_DIR/part2_final_comparison.txt"
    checkpoints=(before_placement after_placement after_opt_prects after_extractrc_postcts after_opt_postcts_hold after_extractrc_preroute after_global_route after_detail_route after_opt_postroute_setup after_opt_postroute_hold after_extractrc_postroute final_postroute)

    {
        echo "run_mode,checkpoint,setup_slack_ns,hold_slack_ns,total_power_mw"
        for mode in timing_driven no_timing_driven; do
            if [ "$mode" = "timing_driven" ]; then
                base="$TD_RUN_DIR/metrics"
            else
                base="$NOTD_RUN_DIR/metrics"
            fi

            for cp in "${checkpoints[@]}"; do
                setup="$(grep -m1 'Slack Time' "$base/${mode}_${cp}_setup.tarpt" 2>/dev/null | awk '{print $NF}' || true)"
                hold="$(grep -m1 'Slack Time' "$base/${mode}_${cp}_hold.tarpt" 2>/dev/null | awk '{print $NF}' || true)"
                power="$(awk '/Total Power:[[:space:]]*[-+]?[0-9]/ {print $3; exit}' "$base/${mode}_${cp}_power.rpt" 2>/dev/null || true)"
                [ -z "$setup" ] && setup="N/A"
                [ -z "$hold" ] && hold="N/A"
                [ -z "$power" ] && power="N/A"
                echo "${mode},${cp},${setup},${hold},${power}"
            done
        done
    } > "$P2_CSV"

    {
        echo "# Lab 4 Part 2 Metrics (Latest Static Runs)"
        echo ""
        echo "Timing-driven run: ${TD_RUN_DIR}"
        echo "No-timing-driven run: ${NOTD_RUN_DIR}"
        echo ""
        echo "| Run Mode | Checkpoint | Setup Slack (ns) | Hold Slack (ns) | Total Power (mW) |"
        echo "|---|---|---:|---:|---:|"
        tail -n +2 "$P2_CSV" | while IFS=, read -r run_mode checkpoint setup_s hold_s power_mw; do
            echo "| ${run_mode} | ${checkpoint} | ${setup_s} | ${hold_s} | ${power_mw} |"
        done
    } > "$P2_MD"

    td_setup="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $3}' "$P2_CSV")"
    td_hold="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $4}' "$P2_CSV")"
    td_power="$(awk -F, '$1=="timing_driven" && $2=="final_postroute" {print $5}' "$P2_CSV")"
    notd_setup="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $3}' "$P2_CSV")"
    notd_hold="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $4}' "$P2_CSV")"
    notd_power="$(awk -F, '$1=="no_timing_driven" && $2=="final_postroute" {print $5}' "$P2_CSV")"

    {
        echo "Lab 4 Part 2 Final Checkpoint Comparison"
        echo "timing_driven: setup=${td_setup} ns, hold=${td_hold} ns, power=${td_power} mW"
        echo "no_timing_driven: setup=${notd_setup} ns, hold=${notd_hold} ns, power=${notd_power} mW"
    } > "$P2_TXT"

    copy_file_if_exists "$P2_CSV" "$SUBMIT_DIR/${PREFIX}_Lab4_2_metrics.csv"
    copy_file_if_exists "$P2_MD" "$SUBMIT_DIR/${PREFIX}_Lab4_2_metrics.md"
    copy_file_if_exists "$P2_TXT" "$SUBMIT_DIR/${PREFIX}_Lab4_2_final_comparison.txt"
else
    echo "MISS: cannot build combined Part 2 metrics (need both latest run markers)"
fi

# Part 3 artifacts if already available.
copy_file_if_exists "$LAB4_DIR/lab4_part3.tcl" "$SUBMIT_DIR/${PREFIX}_Lab4_3.tcl"
P3_INVS="$LAB4_DIR/output/s1494_part3.invs"
P3_INVS_DAT="$LAB4_DIR/output/s1494_part3.invs.dat"
if [ ! -e "$P3_INVS" ]; then
    P3_INVS="$(latest_output_match "s1494_part3.invs")"
fi
if [ ! -e "$P3_INVS_DAT" ]; then
    P3_INVS_DAT="$(latest_output_match "s1494_part3.invs.dat")"
fi
if [ -n "${P3_INVS:-}" ] && [ -e "$P3_INVS" ]; then
    if [ -d "$P3_INVS" ]; then
        copy_dir_if_exists "$P3_INVS" "$SUBMIT_DIR/${PREFIX}_Lab4_3.invs"
    else
        copy_file_if_exists "$P3_INVS" "$SUBMIT_DIR/${PREFIX}_Lab4_3.invs"
    fi
else
    echo "MISS: Part 3 .invs"
fi
if [ -n "${P3_INVS_DAT:-}" ] && [ -e "$P3_INVS_DAT" ]; then
    copy_dir_if_exists "$P3_INVS_DAT" "$SUBMIT_DIR/${PREFIX}_Lab4_3.invs.dat"
else
    echo "MISS: Part 3 .invs.dat"
fi

# Report and submission metadata.
copy_file_if_exists "$LAB4_DIR/results_submission.txt" "$SUBMIT_DIR/${PREFIX}_Lab4_results_submission.txt"
copy_file_if_exists "$LAB4_DIR/documentation/problem2.md" "$SUBMIT_DIR/problem2.md"

DOC_PDF="$(find "$LAB4_DIR/documentation" -maxdepth 1 -type f -name '*.pdf' | sort | tail -n 1 || true)"
if [ -n "$DOC_PDF" ] && [ -f "$DOC_PDF" ]; then
    copy_file_if_exists "$DOC_PDF" "$SUBMIT_DIR/${PREFIX}_Lab4.pdf"
else
    echo "MISS: documentation PDF"
fi

echo ""
echo "=========================================="
echo "  results/submission_ready contents"
echo "=========================================="
find "$SUBMIT_DIR" -maxdepth 2 -mindepth 1 | sort

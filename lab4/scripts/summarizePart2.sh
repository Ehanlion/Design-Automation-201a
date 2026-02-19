#!/bin/bash
# Lab 4 Part 2 -- Side-by-side summary of timing-driven vs non-timing-driven
#
# Reads the report files from both runs and prints a comparison table
# showing worst setup slack, worst hold slack, and total power at each step.
#
# Searches both the new results layout (results/timing_driven/, results/no_timing_driven/)
# and the old layout (results/part2_runs/*/metrics/) so it works before and after re-running.
#
# Usage:  bash scripts/lab4_part2_summary.sh   (from lab4/)

SCRIPTDIR="$(cd "$(dirname "$0")" && pwd)"
LABDIR="$(dirname "$SCRIPTDIR")"

# New-format directories (populated by the rewritten Tcl scripts)
NEW_TDDIR="${LABDIR}/results/timing_driven"
NEW_NOTDDIR="${LABDIR}/results/no_timing_driven"

# Old-format directories (from earlier runs)
OLD_NOTD_METRICS="${LABDIR}/results/part2_runs/no_timing_driven_latest/metrics"
OLD_TD_METRICS="${LABDIR}/results/part2_runs/timing_driven_latest/metrics"

# Each row: "label | new_step_prefix | old_checkpoint | old_run_label_td | old_run_label_notd"
#   new files:  {RESULTSDIR}/{new_step_prefix}_{setup|hold}.tarpt / _power.rpt
#   old files:  {OLD_METRICS}/{old_run_label}_{old_checkpoint}_{setup|hold}.tarpt / _power.rpt
ROWS=(
    "Before Placement|01_before_placement|before_placement"
    "After Placement|02_after_placement|after_placement"
    "After Pre-CTS Opt|03_after_opt_prects|after_opt_prects"
    "After Post-CTS RC Ext|05_after_postcts_rc|after_extractrc_postcts"
    "After Post-CTS Hold Opt|06_after_opt_postcts_hold|after_opt_postcts_hold"
    "After Pre-Route RC Ext|07_after_preroute_rc|after_extractrc_preroute"
    "After Global Route|08_after_global_route|after_global_route"
    "After Detail Route|09_after_detail_route|after_detail_route"
    "After Post-Route Setup Opt|10_after_opt_postroute_setup|after_opt_postroute_setup"
    "After Post-Route Hold Opt|11_after_opt_postroute_hold|after_opt_postroute_hold"
    "Final Post-Route RC Ext|12_final_postroute_rc|after_extractrc_postroute"
)

# Try to find a report file, checking new path first then old path.
#   $1 = "td" or "notd"
#   $2 = new step prefix  (e.g. 01_before_placement)
#   $3 = old checkpoint    (e.g. before_placement)
#   $4 = suffix            (e.g. setup.tarpt, hold.tarpt, power.rpt)
find_report() {
    local mode="$1" new_step="$2" old_ckpt="$3" suffix="$4"

    if [[ "$mode" == "td" ]]; then
        local new_dir="$NEW_TDDIR"
        local old_dir="$OLD_TD_METRICS"
        local old_label="timing_driven"
    else
        local new_dir="$NEW_NOTDDIR"
        local old_dir="$OLD_NOTD_METRICS"
        local old_label="no_timing_driven"
    fi

    local new_file="${new_dir}/${new_step}_${suffix}"
    local old_file="${old_dir}/${old_label}_${old_ckpt}_${suffix}"

    if [[ -f "$new_file" ]]; then
        echo "$new_file"
    elif [[ -f "$old_file" ]]; then
        echo "$old_file"
    else
        echo ""
    fi
}

# Pull the worst (Path 1) slack from an Innovus report_timing file.
get_slack() {
    local file="$1"
    if [[ -z "$file" || ! -f "$file" ]]; then echo "---"; return; fi
    local val
    val=$(grep -m1 "Slack Time" "$file" | grep -oE '[-]?[0-9]+\.[0-9]+')
    if [[ -z "$val" ]]; then echo "---"; else echo "$val"; fi
}

# Pull total power from an Innovus report_power file.
get_power() {
    local file="$1"
    if [[ -z "$file" || ! -f "$file" ]]; then echo "---"; return; fi
    local val
    val=$(grep "^Total Power:" "$file" | head -1 | grep -oE '[0-9]+\.[0-9]+')
    if [[ -z "$val" ]]; then echo "---"; else echo "$val"; fi
}

SEP="--------------------------------------------------------------------------------------------------------------"

echo ""
echo "  Lab 4 Part 2 -- Timing vs Non-Timing-Driven Comparison"
echo ""
printf "%-28s | %10s %10s | %10s %10s | %12s %12s\n" \
       "Step" "Setup TD" "Setup NoTD" "Hold TD" "Hold NoTD" "Power TD" "Power NoTD"
echo "$SEP"

for row in "${ROWS[@]}"; do
    IFS='|' read -r label new_step old_ckpt <<< "$row"

    td_setup_f=$(find_report td   "$new_step" "$old_ckpt" "setup.tarpt")
    notd_setup_f=$(find_report notd "$new_step" "$old_ckpt" "setup.tarpt")
    td_hold_f=$(find_report td   "$new_step" "$old_ckpt" "hold.tarpt")
    notd_hold_f=$(find_report notd "$new_step" "$old_ckpt" "hold.tarpt")
    td_power_f=$(find_report td   "$new_step" "$old_ckpt" "power.rpt")
    notd_power_f=$(find_report notd "$new_step" "$old_ckpt" "power.rpt")

    printf "%-28s | %10s %10s | %10s %10s | %12s %12s\n" \
           "$label" \
           "$(get_slack "$td_setup_f")" "$(get_slack "$notd_setup_f")" \
           "$(get_slack "$td_hold_f")"  "$(get_slack "$notd_hold_f")" \
           "$(get_power "$td_power_f")" "$(get_power "$notd_power_f")"
done

echo "$SEP"
echo ""

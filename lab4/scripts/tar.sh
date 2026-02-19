#!/usr/bin/env bash
set -euo pipefail

# TAR the Lab4 submission files (use submit.sh to submit).
#
# Required final structure:
#   <PREFIX>_Lab4/
#     <PREFIX>_Lab4_1.tcl
#     <PREFIX>_Lab4_1.invs
#     <PREFIX>_Lab4_1.invs.dat/
#     <PREFIX>_Lab4_2.tcl
#     <PREFIX>_Lab4_2.invs
#     <PREFIX>_Lab4_2.invs.dat/
#     <PREFIX>_Lab4_3.tcl
#     <PREFIX>_Lab4_3.invs
#     <PREFIX>_Lab4_3.invs.dat/
#     <PREFIX>_Lab4.pdf
#     <PREFIX>_Lab4_results_submission.txt
#
# Tarball:
#   <PREFIX>_Lab4_pinXXXX.tar.gz

# Configuration
PREFIX="Owen-Ethan_905452983_palatics"
DIR_NAME="${PREFIX}_Lab4"
PIN="1234"  # TODO: Replace with your actual 4-digit PIN before submitting

# Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(dirname "$SCRIPT_DIR")"
SUBMISSIONS_DIR="$LAB4_DIR/submission"
SUBMISSION_DIR_PATH="$SUBMISSIONS_DIR/$DIR_NAME"
TAR_FILE="${DIR_NAME}_pin${PIN}.tar.gz"
TAR_PATH="${SUBMISSIONS_DIR}/${TAR_FILE}"

mkdir -p "$SUBMISSIONS_DIR"

latest_output_match() {
    local pattern="$1"
    if [ ! -d "$LAB4_DIR/output" ]; then
        return
    fi

    local matches=()
    local newest=""

    while IFS= read -r -d '' path; do
        matches+=("$path")
    done < <(find "$LAB4_DIR/output" -maxdepth 3 -name "$pattern" -print0 2>/dev/null)

    if [ "${#matches[@]}" -eq 0 ]; then
        return
    fi

    newest="$(ls -1t "${matches[@]}" 2>/dev/null | head -n 1 || true)"
    printf "%s" "$newest"
}

latest_doc_pdf_match() {
    if [ ! -d "$LAB4_DIR/documentation" ]; then
        return
    fi
    find "$LAB4_DIR/documentation" -maxdepth 1 -type f -name "*.pdf" -print 2>/dev/null | \
        sort | tail -n 1 || true
}

LATEST_PART1_INVS="$(latest_output_match "s1494_part1.invs")"
LATEST_PART1_INVS_DAT="$(latest_output_match "s1494_part1.invs.dat")"
LATEST_PART2_NOTD_INVS="$(latest_output_match "s1494_no_timing_driven.invs")"
LATEST_PART2_NOTD_INVS_DAT="$(latest_output_match "s1494_no_timing_driven.invs.dat")"
LATEST_PART3_INVS="$(latest_output_match "s1494_part3.invs")"
LATEST_PART3_INVS_DAT="$(latest_output_match "s1494_part3.invs.dat")"
LATEST_SUMMARY_RPT="$(latest_output_match "summary.rpt")"
LATEST_SETUP_TARPT="$(latest_output_match "*postrouting_setup*.tarpt")"
LATEST_REPORT_PDF="$(latest_doc_pdf_match)"

update_results_submission_from_part1() {
    local results_file="$LAB4_DIR/results_submission.txt"
    local tcl_source=""
    local summary_source=""
    local setup_source=""
    local initial_util=""
    local final_util=""
    local final_setup_slack=""
    local tmp_file=""
    local candidate=""

    for candidate in \
        "$LAB4_DIR/lab4_part1.tcl" \
        "$LAB4_DIR/lab4_1.tcl" \
        "$LAB4_DIR/${PREFIX}_Lab4_1.tcl" \
        "$SUBMISSIONS_DIR/${PREFIX}_Lab4_1.tcl"; do
        if [ -f "$candidate" ]; then
            tcl_source="$candidate"
            break
        fi
    done

    for candidate in \
        "$LAB4_DIR/output/summary.rpt" \
        "$LATEST_SUMMARY_RPT"; do
        if [ -n "$candidate" ] && [ -f "$candidate" ]; then
            summary_source="$candidate"
            break
        fi
    done

    for candidate in \
        "$LAB4_DIR/output/s1494_postrouting_setup.tarpt" \
        "$LATEST_SETUP_TARPT"; do
        if [ -n "$candidate" ] && [ -f "$candidate" ]; then
            setup_source="$candidate"
            break
        fi
    done

    if [ ! -f "$results_file" ]; then
        echo "ERROR: results file not found: $results_file"
        exit 1
    fi

    if [ -z "$tcl_source" ] || [ -z "$summary_source" ] || [ -z "$setup_source" ]; then
        echo "ERROR: Could not locate all Part 1 sources needed to auto-fill results_submission.txt"
        echo "  TCL source:     ${tcl_source:-MISSING}"
        echo "  Summary report: ${summary_source:-MISSING}"
        echo "  Setup report:   ${setup_source:-MISSING}"
        echo "Run Part 1 first (bash scripts/runLab4Part1.sh), then re-run tar.sh."
        exit 1
    fi

    initial_util="$(sed -n 's/^[[:space:]]*set[[:space:]]\+UTIL[[:space:]]\+\([0-9][0-9.]*\).*/\1/p' "$tcl_source" | head -n 1)"
    # Use "Core Density #2 (Subtracting Physical Cells)" — the real logic
    # utilization after the tool finishes, excluding filler cells.
    # "Effective Utilization" includes fillers and always reads ~1.0.
    local pct
    pct="$(grep 'Core Density #2' "$summary_source" | sed -n 's/.*:[[:space:]]*\([0-9.]*\)%.*/\1/p' | head -n 1)"
    if [ -n "$pct" ]; then
        final_util="$(awk "BEGIN { printf \"%.5f\", $pct / 100 }")"
    else
        final_util=""
    fi
    final_setup_slack="$(grep -m1 'Slack Time' "$setup_source" | awk '{print $NF}' || true)"

    if [ -z "$initial_util" ] || [ -z "$final_util" ] || [ -z "$final_setup_slack" ]; then
        echo "ERROR: Failed to parse one or more required Part 1 values."
        echo "  INITIAL_UTILL (from TCL): ${initial_util:-MISSING}"
        echo "  FINAL_UTIL (from summary): ${final_util:-MISSING}"
        echo "  FINAL_SETUP_SLACK (from setup): ${final_setup_slack:-MISSING}"
        exit 1
    fi

    tmp_file="$(mktemp)"
    awk -v init="$initial_util" -v fin="$final_util" -v setup="$final_setup_slack" '
        /^[[:space:]]*INITIAL_UTILL:/ { $0 = "    INITIAL_UTILL: " init }
        /^[[:space:]]*FINAL_UTIL:/ { $0 = "    FINAL_UTIL: " fin }
        /^[[:space:]]*FINAL_SETUP_SLACK:/ { $0 = "    FINAL_SETUP_SLACK: " setup }
        { print }
    ' "$results_file" > "$tmp_file"

    mv "$tmp_file" "$results_file"

    echo "Updated results_submission.txt from Part 1 reports:"
    echo "  INITIAL_UTILL: $initial_util"
    echo "  FINAL_UTIL: $final_util"
    echo "  FINAL_SETUP_SLACK: $final_setup_slack"
}

# Clean previous output
if [ -d "$SUBMISSION_DIR_PATH" ]; then
    echo "Removing existing directory: $SUBMISSION_DIR_PATH"
    rm -rf "$SUBMISSION_DIR_PATH"
fi
mkdir -p "$SUBMISSION_DIR_PATH"

if [ -f "$TAR_PATH" ]; then
    echo "Removing existing tarball: $TAR_PATH"
    rm -f "$TAR_PATH"
fi

missing_items=()

copy_from_candidates() {
    local dest_name="$1"
    shift
    local dest_path="$SUBMISSION_DIR_PATH/$dest_name"
    local src_path=""
    local candidate=""

    for candidate in "$@"; do
        if [ -z "$candidate" ]; then
            continue
        fi
        if [ -e "$candidate" ]; then
            src_path="$candidate"
            break
        fi
    done

    if [ -z "$src_path" ]; then
        missing_items+=("$dest_name")
        echo "MISS: $dest_name"
        return
    fi

    if [ -d "$src_path" ]; then
        echo "COPY: $src_path -> $dest_path/"
        cp -R "$src_path" "$dest_path"
    else
        echo "COPY: $src_path -> $dest_path"
        cp "$src_path" "$dest_path"
    fi
}

echo "Collecting Lab 4 files..."
update_results_submission_from_part1

copy_from_candidates "${PREFIX}_Lab4_1.tcl" \
    "$LAB4_DIR/lab4_part1.tcl" \
    "$LAB4_DIR/lab4_1.tcl" \
    "$LAB4_DIR/${PREFIX}_Lab4_1.tcl" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_1.tcl"

copy_from_candidates "${PREFIX}_Lab4_1.invs" \
    "$LAB4_DIR/output/s1494_part1.invs" \
    "$LATEST_PART1_INVS" \
    "$LAB4_DIR/${PREFIX}_Lab4_1.invs" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_1.invs"

copy_from_candidates "${PREFIX}_Lab4_1.invs.dat" \
    "$LAB4_DIR/output/s1494_part1.invs.dat" \
    "$LATEST_PART1_INVS_DAT" \
    "$LAB4_DIR/${PREFIX}_Lab4_1.invs.dat" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_1.invs.dat"

copy_from_candidates "${PREFIX}_Lab4_2.tcl" \
    "$LAB4_DIR/lab4_part2_no_timing_driven.tcl" \
    "$LAB4_DIR/lab4_part2_notimingdriven.tcl" \
    "$LAB4_DIR/lab4_part2_noTimingDriven.tcl" \
    "$LAB4_DIR/lab4_part2.tcl" \
    "$LAB4_DIR/lab4_2.tcl" \
    "$LAB4_DIR/${PREFIX}_Lab4_2.tcl" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_2.tcl"

copy_from_candidates "${PREFIX}_Lab4_2.invs" \
    "$LAB4_DIR/results/no_timing_driven/s1494_no_timing_driven.invs" \
    "$LAB4_DIR/output/s1494_no_timing_driven.invs" \
    "$LATEST_PART2_NOTD_INVS" \
    "$LAB4_DIR/${PREFIX}_Lab4_2.invs" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_2.invs"

copy_from_candidates "${PREFIX}_Lab4_2.invs.dat" \
    "$LAB4_DIR/results/no_timing_driven/s1494_no_timing_driven.invs.dat" \
    "$LAB4_DIR/output/s1494_no_timing_driven.invs.dat" \
    "$LATEST_PART2_NOTD_INVS_DAT" \
    "$LAB4_DIR/${PREFIX}_Lab4_2.invs.dat" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_2.invs.dat"

copy_from_candidates "${PREFIX}_Lab4_3.tcl" \
    "$LAB4_DIR/lab4_part3.tcl" \
    "$LAB4_DIR/lab4_3.tcl" \
    "$LAB4_DIR/${PREFIX}_Lab4_3.tcl" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_3.tcl"

copy_from_candidates "${PREFIX}_Lab4_3.invs" \
    "$LAB4_DIR/output/s1494_part3.invs" \
    "$LATEST_PART3_INVS" \
    "$LAB4_DIR/${PREFIX}_Lab4_3.invs" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_3.invs"

copy_from_candidates "${PREFIX}_Lab4_3.invs.dat" \
    "$LAB4_DIR/output/s1494_part3.invs.dat" \
    "$LATEST_PART3_INVS_DAT" \
    "$LAB4_DIR/${PREFIX}_Lab4_3.invs.dat" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_3.invs.dat"

copy_from_candidates "${PREFIX}_Lab4.pdf" \
    "$LAB4_DIR/documentation/${PREFIX}_Lab4.pdf" \
    "$LATEST_REPORT_PDF" \
    "$LAB4_DIR/${PREFIX}_Lab4.pdf" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4.pdf"

copy_from_candidates "${PREFIX}_Lab4_results_submission.txt" \
    "$LAB4_DIR/results_submission.txt" \
    "$LAB4_DIR/${PREFIX}_Lab4_results_submission.txt" \
    "$SUBMISSIONS_DIR/${PREFIX}_Lab4_results_submission.txt"

if [ "${#missing_items[@]}" -ne 0 ]; then
    echo ""
    echo "ERROR: Missing required Lab 4 submission items:"
    for item in "${missing_items[@]}"; do
        echo "  - $item"
    done
    echo ""
    echo "Fill in/create missing files, then re-run: bash scripts/tar.sh"
    exit 1
fi

echo ""
echo "Setting permissions (chmod -R go+rx)..."
chmod -R go+rx "$SUBMISSION_DIR_PATH"

echo "Creating tarball: $TAR_PATH"
(
    cd "$SUBMISSIONS_DIR"
    tar -czf "$TAR_FILE" "$DIR_NAME"
)
chmod go+rx "$TAR_PATH"

echo ""
echo "========================================="
echo "Tarball created successfully"
echo "========================================="
echo "File: $TAR_PATH"
echo "Size: $(du -h "$TAR_PATH" | awk '{print $1}')"
echo ""
echo "Contents:"
tar -tzf "$TAR_PATH"
echo ""
echo "To submit, run: bash scripts/submit.sh"

#!/usr/bin/env bash
set -euo pipefail

# Runs Lab 4 Part 2 non-timing-driven flow.
# Usage:
#   ./scripts/runLab4Part2NoTD.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PART2_TCL="${LAB4_DIR}/lab4_part2_no_timing_driven.tcl"
RESULTS_DIR="${LAB4_DIR}/results/no_timing_driven"
RUN_ID="$(date +%Y%m%d_%H%M%S)_$$"
CONSOLE_LOG="${LAB4_DIR}/output/innovus_part2_notd_console_${RUN_ID}.log"

if [ ! -f "$PART2_TCL" ]; then
    echo "Error: Tcl script not found: $PART2_TCL"
    exit 1
fi

mkdir -p "${LAB4_DIR}/output"

echo "=========================================="
echo "  Lab 4 Part 2 — No Timing-Driven"
echo "=========================================="
echo "  Tcl script:   $PART2_TCL"
echo "  Results dir:  $RESULTS_DIR"
echo "  Console log:  $CONSOLE_LOG"
echo "=========================================="
echo ""

# Source Cadence/Mentor environment if innovus is not already in PATH
if ! command -v innovus >/dev/null 2>&1; then
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
fi

if ! command -v innovus >/dev/null 2>&1; then
    echo "Error: innovus not found after sourcing SETUP files."
    exit 1
fi

echo "Using innovus: $(which innovus)"
echo ""

INNOVUS_EXIT=0
(
    cd "$LAB4_DIR"
    innovus -batch -no_gui -init "$PART2_TCL"
) 2>&1 | tee "$CONSOLE_LOG" || INNOVUS_EXIT=$?

if [ "$INNOVUS_EXIT" -ne 0 ]; then
    echo ""
    echo "ERROR: Innovus run failed with exit code $INNOVUS_EXIT"
    echo "Log: $CONSOLE_LOG"
    exit "$INNOVUS_EXIT"
fi

# Verify the expected outputs exist
INVS_FILE="${RESULTS_DIR}/s1494_no_timing_driven.invs"
INVS_DAT_DIR="${RESULTS_DIR}/s1494_no_timing_driven.invs.dat"
if [ ! -e "$INVS_FILE" ] || [ ! -d "$INVS_DAT_DIR" ]; then
    echo "ERROR: Missing required Part 2 no-TD artifacts:"
    echo "  $INVS_FILE"
    echo "  $INVS_DAT_DIR"
    exit 3
fi

# Copy console log into results directory for easy reference
cp "$CONSOLE_LOG" "${RESULTS_DIR}/innovus_console.log"

echo ""
echo "=========================================="
echo "  Part 2 no-TD run complete"
echo "=========================================="
echo "  Results dir:  $RESULTS_DIR"
echo "  Console log:  $CONSOLE_LOG"
echo "  Design DB:    $INVS_FILE"
echo "  Design data:  $INVS_DAT_DIR"
echo "=========================================="

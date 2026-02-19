#!/usr/bin/env bash
set -euo pipefail

# Runs the static lab4_part1.tcl submission script.
# All values (utilization, output dir) are hardcoded in the Tcl file.
#
# Usage:
#   ./scripts/runLab4Part1.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PART1_TCL="${LAB4_DIR}/lab4_part1.tcl"
CONSOLE_LOG="${LAB4_DIR}/output/innovus_part1_console.log"

if [ ! -f "$PART1_TCL" ]; then
    echo "Error: Tcl script not found: $PART1_TCL"
    exit 1
fi

echo "=========================================="
echo "  Lab 4 Part 1 — submission run"
echo "=========================================="
echo "  Tcl script: $PART1_TCL"
echo "  Working dir: $LAB4_DIR"
echo "  Console log: $CONSOLE_LOG"
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

mkdir -p "${LAB4_DIR}/output"

INNOVUS_EXIT=0
(
    cd "$LAB4_DIR"
    innovus -batch -no_gui -init lab4_part1.tcl
) 2>&1 | tee "$CONSOLE_LOG" || INNOVUS_EXIT=$?

echo ""
echo "=========================================="
echo "  Run finished (exit code: $INNOVUS_EXIT)"
echo "  Console log: $CONSOLE_LOG"
echo "=========================================="

# Auto-extract results
EXTRACT="${SCRIPT_DIR}/extract_results.sh"
if [ -f "$EXTRACT" ] && [ -x "$EXTRACT" ]; then
    echo ""
    "$EXTRACT" "${LAB4_DIR}/output"
fi

# Show submission-relevant files
echo ""
echo "=========================================="
echo "  Submission files (Part 1)"
echo "=========================================="
OUTDIR="${LAB4_DIR}/output"
for f in "${OUTDIR}/s1494_part1.invs" "${OUTDIR}/s1494_part1.invs.dat"; do
    if [ -e "$f" ]; then
        echo "  OK:    $(basename "$f")"
    else
        echo "  MISS:  $(basename "$f")"
    fi
done
echo "  Script: lab4_part1.tcl"
echo "=========================================="

# Stage submission-ready artifacts into lab4/results.
UPDATE_RESULTS="${SCRIPT_DIR}/updateResultsDir.sh"
if [ -f "$UPDATE_RESULTS" ] && [ -x "$UPDATE_RESULTS" ]; then
    echo ""
    "$UPDATE_RESULTS"
fi

exit "$INNOVUS_EXIT"

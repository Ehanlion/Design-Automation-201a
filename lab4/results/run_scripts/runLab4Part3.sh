#!/usr/bin/env bash
set -euo pipefail

# Runs the static final Lab 4 Part 3 submission script.
# The Tcl script has inlined final settings (util/stripes/output directory).
#
# Usage:
#   ./scripts/runLab4Part3.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PART3_TCL="${LAB4_DIR}/lab4_part3.tcl"

if [ ! -f "$PART3_TCL" ]; then
    echo "Error: Tcl script not found: $PART3_TCL"
    exit 1
fi

if [ "${1:-}" != "" ]; then
    echo "Error: this submission runner takes no arguments."
    echo "Run: ./scripts/runLab4Part3.sh"
    exit 1
fi

OUT_DIR="${LAB4_DIR}/results/part3_final_with_stripes"

mkdir -p "$OUT_DIR"
CONSOLE_LOG="${OUT_DIR}/innovus_part3_console.log"

echo "=========================================="
echo "  Lab 4 Part 3 — submission run"
echo "=========================================="
echo "  Tcl script:   $PART3_TCL"
echo "  Output dir:   $OUT_DIR"
echo "  Console log:  $CONSOLE_LOG"
echo "=========================================="
echo ""

# Source Cadence/Mentor environment if innovus is not already in PATH.
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
    innovus -batch -no_gui -init "$PART3_TCL"
) 2>&1 | tee "$CONSOLE_LOG" || INNOVUS_EXIT=$?

echo ""
echo "=========================================="
echo "  Run finished (exit code: $INNOVUS_EXIT)"
echo "  Output dir:  $OUT_DIR"
echo "  Console log: $CONSOLE_LOG"
echo "=========================================="

EXTRACT="${SCRIPT_DIR}/extract_results.sh"
if [ -f "$EXTRACT" ] && [ -x "$EXTRACT" ]; then
    echo ""
    "$EXTRACT" "$OUT_DIR"
fi

echo ""
echo "=========================================="
echo "  Part 3 submission artifacts"
echo "=========================================="
for f in "${OUT_DIR}/s1494_part3.invs" "${OUT_DIR}/s1494_part3.invs.dat" "${OUT_DIR}/s1494_postrouting_power.rpt"; do
    if [ -e "$f" ]; then
        echo "  OK:    $(basename "$f")"
    else
        echo "  MISS:  $(basename "$f")"
    fi
done
echo "=========================================="

UPDATE_RESULTS="${SCRIPT_DIR}/updateResultsDir.sh"
if [ -f "$UPDATE_RESULTS" ] && [ -x "$UPDATE_RESULTS" ]; then
    echo ""
    "$UPDATE_RESULTS"
fi

exit "$INNOVUS_EXIT"

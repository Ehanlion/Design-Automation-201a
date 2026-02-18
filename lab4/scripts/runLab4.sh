#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ./scripts/runLab4.sh [target_utilization]
# Example:
#   ./scripts/runLab4.sh 0.90

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
TCL_SCRIPT="${LAB4_DIR}/lab4_skeleton.tcl"

UTIL="${1:-0.991}"
if ! awk "BEGIN { exit !(${UTIL} > 0.0 && ${UTIL} < 1.0) }"; then
    echo "Error: utilization must be strictly between 0 and 1."
    exit 1
fi

if [ ! -f "$TCL_SCRIPT" ]; then
    echo "Error: Tcl script not found: $TCL_SCRIPT"
    exit 1
fi

RUN_ID="$(date +%Y%m%d_%H%M%S)_$$"
OUT_DIR="${LAB4_DIR}/output/run_${RUN_ID}_util${UTIL}"
CONSOLE_LOG="${OUT_DIR}/innovus_console.log"
mkdir -p "$OUT_DIR"

echo "=========================================="
echo "  Lab 4 Run"
echo "=========================================="
echo "  Tcl script:       $TCL_SCRIPT"
echo "  Target util:      $UTIL"
echo "  Run id:           $RUN_ID"
echo "  Output dir:       $OUT_DIR"
echo "  Console log:      $CONSOLE_LOG"
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

export LAB4_OUTPUTDIR="$OUT_DIR"
export LAB4_UTIL="$UTIL"

INNOVUS_EXIT=0
(
    cd "$LAB4_DIR"
    innovus -batch -no_gui -init "$TCL_SCRIPT"
) 2>&1 | tee "$CONSOLE_LOG" || INNOVUS_EXIT=$?

echo ""
echo "=========================================="
echo "  Run finished (exit code: $INNOVUS_EXIT)"
echo "  Output dir:  $OUT_DIR"
echo "  Console log: $CONSOLE_LOG"
echo "=========================================="

# Auto-extract results if the extraction script exists
EXTRACT="${SCRIPT_DIR}/extract_results.sh"
if [ -f "$EXTRACT" ] && [ -x "$EXTRACT" ]; then
    echo ""
    "$EXTRACT" "$OUT_DIR"
fi

exit "$INNOVUS_EXIT"

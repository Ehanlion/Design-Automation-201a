#!/usr/bin/env bash
set -euo pipefail

# Runs Lab 4 Part 3 (power-stripe flow).
#
# Usage:
#   ./scripts/runLab4Part3.sh [target_utilization] [output_dir] [enable_stripes]
#
# Examples:
#   ./scripts/runLab4Part3.sh
#   ./scripts/runLab4Part3.sh 0.990
#   ./scripts/runLab4Part3.sh 0.990 output
#   ./scripts/runLab4Part3.sh 0.915 output 1
#   ./scripts/runLab4Part3.sh 0.915 output/run_part3_nostripes 0
#
# Notes:
# - If output_dir is omitted, a timestamped run directory is created under output/.
# - Use output_dir=output to place canonical submission files in lab4/output/.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PART3_TCL="${LAB4_DIR}/lab4_part3.tcl"

UTIL="${1:-0.990}"
if ! awk "BEGIN { exit !(${UTIL} > 0.0 && ${UTIL} < 1.0) }"; then
    echo "Error: utilization must be strictly between 0 and 1."
    exit 1
fi

if [ ! -f "$PART3_TCL" ]; then
    echo "Error: Tcl script not found: $PART3_TCL"
    exit 1
fi

ENABLE_STRIPES="${3:-1}"
if [ "$ENABLE_STRIPES" != "0" ] && [ "$ENABLE_STRIPES" != "1" ]; then
    echo "Error: enable_stripes must be 0 or 1."
    exit 1
fi

if [ "${2:-}" = "" ]; then
    RUN_ID="$(date +%Y%m%d_%H%M%S)_$$"
    OUT_DIR="${LAB4_DIR}/output/run_part3_${RUN_ID}_util${UTIL}"
else
    if [[ "$2" = /* ]]; then
        OUT_DIR="$2"
    else
        OUT_DIR="${LAB4_DIR}/$2"
    fi
fi

CONSOLE_LOG="${OUT_DIR}/innovus_part3_console.log"
mkdir -p "$OUT_DIR"

echo "=========================================="
echo "  Lab 4 Part 3 Run"
echo "=========================================="
echo "  Tcl script:       $PART3_TCL"
echo "  Target util:      $UTIL"
echo "  Stripes enabled:  $ENABLE_STRIPES"
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

export LAB4_UTIL="$UTIL"
export LAB4_OUTPUTDIR="$OUT_DIR"
export LAB4_ENABLE_STRIPES="$ENABLE_STRIPES"

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
echo "  Part 3 artifacts"
echo "=========================================="
for f in "${OUT_DIR}/s1494_part3.invs" "${OUT_DIR}/s1494_part3.invs.dat" "${OUT_DIR}/s1494_postrouting_power.rpt"; do
    if [ -e "$f" ]; then
        echo "  OK:    $(basename "$f")"
    else
        echo "  MISS:  $(basename "$f")"
    fi
done
echo "=========================================="

# Stage submission-ready artifacts into lab4/results.
UPDATE_RESULTS="${SCRIPT_DIR}/updateResultsDir.sh"
if [ -f "$UPDATE_RESULTS" ] && [ -x "$UPDATE_RESULTS" ]; then
    echo ""
    "$UPDATE_RESULTS"
fi

exit "$INNOVUS_EXIT"

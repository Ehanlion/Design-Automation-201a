#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 3A: Extract Timing Data         */
#*                                                  */
#* This script extracts clock period, total area,  */
#* instances, and slack from 3A files and appends  */
#* to CSV file in format:                          */
#* period,total area,instances,slack               */
#*                                                  */
#* Usage: ./scripts/extractTimingData3A.sh         */
#*        or: bash scripts/extractTimingData3A.sh */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Lab 2 - Problem 3A: Extract Timing Data"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo ""

# Check if Python script exists
PYTHON_SCRIPT="$SCRIPT_DIR/python/extract_timing_data_3A.py"
if [ ! -f "$PYTHON_SCRIPT" ]; then
    echo "Error: Python script not found at $PYTHON_SCRIPT"
    exit 1
fi

# Check if python3 is available
if ! command -v python3 &> /dev/null; then
    echo "Error: 'python3' command not found in PATH!"
    exit 1
fi

# Run the Python script
# If period parameter provided, pass it to the script
if [ $# -gt 0 ]; then
    PERIOD="$1"
    echo "Extracting timing data from 3A files..."
    echo "  - Period: ${PERIOD} ps (from parameter)"
    echo "  - Total area and instances from 3A_synth_report_gates.txt"
    echo "  - Slack from 3A_synth_report_timing.txt"
    echo ""
    python3 "$PYTHON_SCRIPT" "${PERIOD}"
else
    echo "Extracting timing data from 3A files..."
    echo "  - Period from lab2_3A.tcl"
    echo "  - Total area and instances from 3A_synth_report_gates.txt"
    echo "  - Slack from 3A_synth_report_timing.txt"
    echo ""
    python3 "$PYTHON_SCRIPT"
fi
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "  Timing data extraction completed!"
    echo "========================================"
else
    echo ""
    echo "========================================"
    echo "  Error: Timing data extraction failed!"
    echo "========================================"
fi

exit $EXIT_CODE

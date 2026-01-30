#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 1: Run TCL Practice Script      */
#*                                                  */
#* This script runs the Problem 1 TCL script       */
#*                                                  */
#* Usage: ./scripts/run_problem1.sh                */
#*        or: bash scripts/run_problem1.sh         */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Lab 2 - Problem 1: TCL Practice"
echo "========================================"
echo ""
echo "Running TCL script from: $LAB2_DIR"
echo ""

# Check if wirelength.txt exists
if [ ! -f "wirelength.txt" ]; then
    echo "Error: wirelength.txt not found in $LAB2_DIR"
    echo "Please ensure the file is present."
    exit 1
fi

# Run the TCL script
echo "Executing: tclsh lab2_problem1.tcl"
echo ""

tclsh lab2_problem1.tcl

# Check exit status
if [ $? -eq 0 ]; then
    echo ""
    echo "Script completed successfully!"
    echo ""
else
    echo ""
    echo "Error: Script failed with exit code $?"
    echo ""
    exit 1
fi

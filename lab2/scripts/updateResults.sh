#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Update Results Submission File          */
#*                                                  */
#* This script updates results_submission.txt     */
#* with values from problem1_results.txt          */
#*                                                  */
#* Usage: ./scripts/update_results.sh              */
#*        or: bash scripts/update_results.sh       */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Updating results_submission.txt"
echo "========================================"
echo ""

# Check if problem1_results.txt exists
if [ ! -f "problem1_results.txt" ]; then
    echo "Error: problem1_results.txt not found in $LAB2_DIR"
    echo "Please ensure the file is present."
    exit 1
fi

# Check if results_submission.txt exists
if [ ! -f "results_submission.txt" ]; then
    echo "Error: results_submission.txt not found in $LAB2_DIR"
    echo "Please ensure the file is present."
    exit 1
fi

echo "Extracting values from problem1_results.txt..."

# Extract values from problem1_results.txt
AVG_WIRELENGTH=$(grep "AVERAGE_WIRELENGTH:" problem1_results.txt | awk '{print $2}')
WIRES_OVER_50=$(grep "NUMBER_OF_WIRES_OVER_50:" problem1_results.txt | awk '{print $2}')

# Update results_submission.txt if values were extracted
if [ -n "$AVG_WIRELENGTH" ] && [ -n "$WIRES_OVER_50" ]; then
    echo "Updating results_submission.txt..."
    echo "  AVERAGE_WIRELENGTH: $AVG_WIRELENGTH"
    echo "  NUMBER_OF_WIRES_OVER_50: $WIRES_OVER_50"
    
    # Update AVERAGE_WIRELENGTH line (preserving format)
    sed -i "s/    AVERAGE_WIRELENGTH:.*/    AVERAGE_WIRELENGTH: $AVG_WIRELENGTH/" results_submission.txt
    # Update NUMBER_OF_WIRES_OVER_50 line (preserving format)
    sed -i "s/    NUMBER_OF_WIRES_OVER_50:.*/    NUMBER_OF_WIRES_OVER_50: $WIRES_OVER_50/" results_submission.txt
    
    echo ""
    echo "Results updated successfully!"
else
    echo "Error: Could not extract values from problem1_results.txt"
    echo "  AVG_WIRELENGTH: '$AVG_WIRELENGTH'"
    echo "  WIRES_OVER_50: '$WIRES_OVER_50'"
    exit 1
fi

echo ""
echo "Extracting values from TCL files for Problem 2..."

# Check if TCL files exist
if [ ! -f "lab2_2A.tcl" ]; then
    echo "Warning: lab2_2A.tcl not found in $LAB2_DIR"
    echo "Skipping Problem 2A update."
else
    # Extract clock period from lab2_2A.tcl (BEST_CLOCK_PERIOD)
    BEST_CLOCK_PERIOD=$(grep "^set clk_period" lab2_2A.tcl | awk '{print $3}')
    
    if [ -n "$BEST_CLOCK_PERIOD" ]; then
        echo "  BEST_CLOCK_PERIOD: $BEST_CLOCK_PERIOD"
        # Update BEST_CLOCK_PERIOD line (preserving format)
        sed -i "s/    A) BEST_CLOCK_PERIOD:.*/    A) BEST_CLOCK_PERIOD: $BEST_CLOCK_PERIOD/" results_submission.txt
    else
        echo "Warning: Could not extract BEST_CLOCK_PERIOD from lab2_2A.tcl"
    fi
fi

if [ ! -f "lab2_2B.tcl" ]; then
    echo "Warning: lab2_2B.tcl not found in $LAB2_DIR"
    echo "Skipping Problem 2B update."
else
    # Extract clock period from lab2_2B.tcl (NEW_BEST_CLOCK_PERIOD)
    NEW_BEST_CLOCK_PERIOD=$(grep "^set clk_period" lab2_2B.tcl | awk '{print $3}')
    
    if [ -n "$NEW_BEST_CLOCK_PERIOD" ]; then
        echo "  NEW_BEST_CLOCK_PERIOD: $NEW_BEST_CLOCK_PERIOD"
        # Update NEW_BEST_CLOCK_PERIOD line (preserving format)
        sed -i "s/    B) NEW_BEST_CLOCK_PERIOD:.*/    B) NEW_BEST_CLOCK_PERIOD: $NEW_BEST_CLOCK_PERIOD/" results_submission.txt
    else
        echo "Warning: Could not extract NEW_BEST_CLOCK_PERIOD from lab2_2B.tcl"
    fi
fi

echo ""

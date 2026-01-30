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

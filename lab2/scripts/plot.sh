#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Area vs Delay Plotting Utility          */
#*                                                  */
#* This script compiles and runs the plotting      */
#* utility to generate area vs delay plots         */
#*                                                  */
#* Usage: ./scripts/plotAreaDelay.sh               */
#*        or: bash scripts/plotAreaDelay.sh        */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Lab 2 - Area vs Delay Plotting Utility"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo ""

# Check if CSV data file exists
if [ ! -f "results/3A_plotting_data.csv" ]; then
    echo "Error: CSV data file 'results/3A_plotting_data.csv' not found!"
    echo "       Please run Problem 3A synthesis first to generate the data."
    exit 1
fi

# Compile the plotting program
echo "Compiling plotting program..."
g++ -std=c++11 -o plot_area_delay plot_area_delay.cpp

# Check if compilation succeeded
if [ $? -ne 0 ]; then
    echo "Error: Compilation failed!"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Run the plotting program
echo "Running plotting program..."
./plot_area_delay

# Check if execution succeeded
if [ $? -ne 0 ]; then
    echo "Error: Plotting program failed!"
    exit 1
fi

echo ""
echo "========================================"
echo "  Success!"
echo "========================================"
echo ""
echo "Generated plots are located in: plotting/"
echo ""

echo ""

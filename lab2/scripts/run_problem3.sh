#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 3: Area-Delay & Power Optimization */
#*                                                  */
#* This script runs Genus synthesis for Problem 3  */
#*                                                  */
#* Usage: ./scripts/run_problem3.sh [tcl_file]    */
#*        or: bash scripts/run_problem3.sh         */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

# Default TCL script
TCL_SCRIPT="tcl_scripts/lab2_problem3.tcl"

# If argument provided, use it as TCL script
if [ $# -gt 0 ]; then
    TCL_SCRIPT="$1"
fi

echo "========================================"
echo "  Lab 2 - Problem 3: Area-Delay & Power Optimization"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo "TCL script: $TCL_SCRIPT"
echo ""

# Check if TCL script exists
if [ ! -f "$TCL_SCRIPT" ]; then
    echo "Error: TCL script '$TCL_SCRIPT' not found!"
    echo "       Creating from skeleton..."
    cp tcl_scripts/lab2_skeleton.tcl "$TCL_SCRIPT"
    echo "       Created $TCL_SCRIPT"
    echo ""
fi

# Check if library file exists
if [ ! -f "NangateOpenCellLibrary_typical.lib" ]; then
    echo "Error: NangateOpenCellLibrary_typical.lib not found!"
    exit 1
fi

# Check if Verilog file exists
if [ ! -f "s15850.v" ]; then
    echo "Error: s15850.v not found!"
    exit 1
fi

# Check if results directory exists
if [ ! -d "results" ]; then
    echo "Creating results directory..."
    mkdir -p results
fi

# Source the setup script if it exists
SETUP_SCRIPT="_setup/new_bash_ee201a_setup"
if [ -f "$SETUP_SCRIPT" ]; then
    echo "Sourcing setup script: $SETUP_SCRIPT"
    source "$SETUP_SCRIPT"
    echo ""
else
    echo "Warning: Setup script '$SETUP_SCRIPT' not found!"
    echo "         Genus may not be available in PATH"
    echo ""
fi

# Check if genus is available
if ! command -v genus &> /dev/null; then
    echo "Error: 'genus' command not found in PATH!"
    echo "       Please ensure the setup script is sourced properly"
    echo "       or source it manually:"
    echo "       source _setup/new_bash_ee201a_setup"
    echo ""
    exit 1
fi

# Run Genus in batch mode (headless)
echo "Running Genus synthesis for Problem 3 (this may take a few minutes)..."
echo ""
echo "Command: genus -batch -no_gui -files $TCL_SCRIPT"
echo ""

genus -batch -no_gui -files "$TCL_SCRIPT"

# Check exit status
if [ $? -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "  Problem 3 synthesis completed successfully!"
    echo "========================================"
    echo ""
    echo "Check results/ directory for results:"
    echo "  - synth_report_timing.txt"
    echo "  - synth_report_gates.txt"
    echo "  - synth_report_power.txt"
    echo "  - s15850_synth.v"
    echo "  - s15850.sdc"
    echo ""
else
    echo ""
    echo "========================================"
    echo "  Error: Synthesis failed!"
    echo "========================================"
    echo ""
    echo "Check genus.log for details"
    echo ""
    exit 1
fi

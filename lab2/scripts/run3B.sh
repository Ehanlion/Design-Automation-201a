#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 3B: Area-Delay & Power Optimization */
#*                                                  */
#* This script runs Genus synthesis for Problem 3B */
#*                                                  */
#* Usage: ./scripts/run3B.sh [tcl_file]            */
#*        or: bash scripts/run3B.sh                */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

# Default TCL script
TCL_SCRIPT="lab2_3B.tcl"

# If argument provided, use it as TCL script
if [ $# -gt 0 ]; then
    TCL_SCRIPT="$1"
fi

echo "========================================"
echo "  Lab 2 - Problem 3B: Area-Delay & Power Optimization"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo "TCL script: $TCL_SCRIPT"
echo ""

# Check if TCL script exists
if [ ! -f "$TCL_SCRIPT" ]; then
    echo "Error: TCL script '$TCL_SCRIPT' not found!"
    echo "       Creating from skeleton..."
    cp lab2_base.tcl "$TCL_SCRIPT"
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

# Source the setup script if it exists
SETUP_SCRIPT="new_bash_ee201a_setup"
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
    echo "       source new_bash_ee201a_setup"
    echo ""
    exit 1
fi

# Run Genus in batch mode (headless)
echo "Running Genus synthesis for Problem 3B (this may take a few minutes)..."
echo ""
echo "Command: genus -batch -no_gui -files $TCL_SCRIPT"
echo ""

genus -batch -no_gui -files "$TCL_SCRIPT"
GENUS_EXIT_CODE=$?

# Check exit status
if [ $GENUS_EXIT_CODE -eq 0 ]; then
    echo ""
    echo "========================================"
    echo "  Problem 3B synthesis completed successfully!"
    echo "========================================"
    echo ""
    
    # Create results directory if it doesn't exist
    RESULTS_DIR="$LAB2_DIR/results"
    mkdir -p "$RESULTS_DIR"
    
    # Copy result files to results directory with prefix
    PREFIX="3B_"
    echo "Copying results to $RESULTS_DIR/ with prefix '$PREFIX'..."
    
    if [ -f "synth_report_timing.txt" ]; then
        cp "synth_report_timing.txt" "$RESULTS_DIR/${PREFIX}synth_report_timing.txt"
        rm "synth_report_timing.txt"
    fi
    
    if [ -f "synth_report_gates.txt" ]; then
        cp "synth_report_gates.txt" "$RESULTS_DIR/${PREFIX}synth_report_gates.txt"
        rm "synth_report_gates.txt"
    fi
    
    if [ -f "synth_report_power.txt" ]; then
        cp "synth_report_power.txt" "$RESULTS_DIR/${PREFIX}synth_report_power.txt"
        rm "synth_report_power.txt"
    fi
    
    if [ -f "s15850_synth.v" ]; then
        cp "s15850_synth.v" "$RESULTS_DIR/${PREFIX}s15850_synth.v"
        rm "s15850_synth.v"
    fi
    
    if [ -f "s15850.sdc" ]; then
        cp "s15850.sdc" "$RESULTS_DIR/${PREFIX}s15850.sdc"
        rm "s15850.sdc"
    fi
    
    echo "Results saved to $RESULTS_DIR/:"
    echo "  - ${PREFIX}synth_report_timing.txt"
    echo "  - ${PREFIX}synth_report_gates.txt"
    echo "  - ${PREFIX}synth_report_power.txt"
    echo "  - ${PREFIX}s15850_synth.v"
    echo "  - ${PREFIX}s15850.sdc"
    echo ""
else
    echo ""
    echo "========================================"
    echo "  Error: Synthesis failed!"
    echo "========================================"
    echo ""
    echo "Check genus.log for details"
    echo ""
fi

# Clean up Genus-generated files
echo "Cleaning up Genus-generated files..."
rm -f genus.log* genus.cmd* genus_* .genus* .cadence
echo "Cleanup complete!"

# Exit with the original exit code
if [ $GENUS_EXIT_CODE -ne 0 ]; then
    exit $GENUS_EXIT_CODE
fi

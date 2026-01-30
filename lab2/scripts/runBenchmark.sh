#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Generate Benchmark Outputs               */
#*                                                  */
#* This script runs lab2_skeleton.tcl as-is        */
#* and routes outputs to _benchmark directory      */
#*                                                  */
#* Usage: ./scripts/run_benchmark.sh               */
#*        or: bash scripts/run_benchmark.sh         */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Lab 2 - Generating Benchmark Outputs"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo ""

# Check if skeleton TCL script exists
if [ ! -f "lab2_skeleton.tcl" ]; then
    echo "Error: lab2_skeleton.tcl not found!"
    exit 1
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

# Create benchmark directory
BENCHMARK_DIR="_benchmark"
if [ -d "$BENCHMARK_DIR" ]; then
    echo "Warning: $BENCHMARK_DIR directory already exists"
    echo "         Removing old benchmark files..."
    rm -rf "$BENCHMARK_DIR"/*
else
    echo "Creating benchmark directory: $BENCHMARK_DIR"
    mkdir -p "$BENCHMARK_DIR"
fi
echo ""

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

# Run Genus in batch mode (headless) with skeleton TCL
echo "Running Genus synthesis with lab2_skeleton.tcl..."
echo "This may take a few minutes..."
echo ""
echo "Command: genus -batch -no_gui -files lab2_skeleton.tcl"
echo ""

genus -batch -no_gui -files lab2_skeleton.tcl

# Check exit status
if [ $? -eq 0 ]; then
    echo ""
    echo "Synthesis completed successfully!"
    echo ""
    echo "Collecting benchmark outputs..."
    
    # List of expected output files from skeleton TCL
    OUTPUT_FILES=(
        "synth_report_timing.txt"
        "synth_report_gates.txt"
        "synth_report_power.txt"
        "s15850_synth.v"
        "s15850.sdc"
    )
    
    # Copy output files to benchmark directory
    FILES_COPIED=0
    for file in "${OUTPUT_FILES[@]}"; do
        if [ -f "$file" ]; then
            cp "$file" "$BENCHMARK_DIR/"
            echo "  ✓ Copied $file"
            ((FILES_COPIED++))
        else
            echo "  ✗ Warning: $file not found"
        fi
    done
    
    # Also copy any log files
    if [ -f "genus.log" ]; then
        cp "genus.log" "$BENCHMARK_DIR/"
        echo "  ✓ Copied genus.log"
    fi
    
    echo ""
    echo "========================================"
    echo "  Benchmark generation completed!"
    echo "========================================"
    echo ""
    echo "Benchmark files saved to: $BENCHMARK_DIR/"
    echo "Files copied: $FILES_COPIED"
    echo ""
    echo "Benchmark outputs:"
    for file in "${OUTPUT_FILES[@]}"; do
        if [ -f "$BENCHMARK_DIR/$file" ]; then
            echo "  - $file"
        fi
    done
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

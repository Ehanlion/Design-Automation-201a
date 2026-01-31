#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 3A: Area-Delay & Power Optimization */
#*                                                  */
#* This script runs Genus synthesis for Problem 3A */
#* with multiple clock period values                */
#*                                                  */
#* Usage: ./scripts/run3Aextract.sh [period1] [period2] ... */
#*        e.g.: ./scripts/run3Aextract.sh 100 200 300 400 500 */
#*        or: bash scripts/run3Aextract.sh 100 200 300        */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

# Default TCL script
TCL_SCRIPT="lab2_3A.tcl"

# Check if any arguments provided
if [ $# -eq 0 ]; then
    echo "Error: No clock period values provided!"
    echo ""
    echo "Usage: ./scripts/run3Aextract.sh [period1] [period2] ..."
    echo "       e.g.: ./scripts/run3Aextract.sh 100 200 300 400 500"
    echo ""
    exit 1
fi

# Store clock periods from command-line arguments
CLOCK_PERIODS=("$@")

echo "========================================"
echo "  Lab 2 - Problem 3A: Area-Delay & Power Optimization"
echo "========================================"
echo ""
echo "Working directory: $LAB2_DIR"
echo "TCL script: $TCL_SCRIPT"
echo "Clock periods to test: ${CLOCK_PERIODS[*]}"
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

# Create results directory if it doesn't exist
RESULTS_DIR="$LAB2_DIR/results"
mkdir -p "$RESULTS_DIR"
PREFIX="3A_"

# Loop through each clock period
TOTAL_PERIODS=${#CLOCK_PERIODS[@]}
PERIOD_NUM=0
FAILED_PERIODS=()

for CLK_PERIOD in "${CLOCK_PERIODS[@]}"; do
    PERIOD_NUM=$((PERIOD_NUM + 1))
    
    echo ""
    echo "========================================"
    echo "  Processing period $PERIOD_NUM of $TOTAL_PERIODS: ${CLK_PERIOD} ps"
    echo "========================================"
    echo ""
    
    # Create temporary TCL script with modified clock period
    TEMP_TCL="${TCL_SCRIPT}.tmp.${CLK_PERIOD}"
    
    # Copy original TCL script and replace clk_period value
    sed "s/^set clk_period [0-9]*/set clk_period ${CLK_PERIOD}/" "$TCL_SCRIPT" > "$TEMP_TCL"
    
    if [ ! -f "$TEMP_TCL" ]; then
        echo "Error: Failed to create temporary TCL script!"
        FAILED_PERIODS+=("${CLK_PERIOD}")
        continue
    fi
    
    # Run Genus in batch mode (headless)
    echo "Running Genus synthesis with clock period ${CLK_PERIOD} ps..."
    echo "Command: genus -batch -no_gui -files $TEMP_TCL"
    echo ""
    
    genus -batch -no_gui -files "$TEMP_TCL"
    GENUS_EXIT_CODE=$?
    
    # Check exit status
    if [ $GENUS_EXIT_CODE -eq 0 ]; then
        echo ""
        echo "  Synthesis completed successfully for period ${CLK_PERIOD} ps!"
        echo ""
        
        # Copy result files to results directory with prefix
        echo "  Copying results to $RESULTS_DIR/ with prefix '$PREFIX'..."
        
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
        
        # Extract timing data for this period
        echo ""
        echo "  Extracting timing data for period ${CLK_PERIOD} ps..."
        echo ""
        
        # Run the extraction script with the period parameter
        bash "$SCRIPT_DIR/extractTimingData3A.sh" "${CLK_PERIOD}"
        EXTRACT_EXIT_CODE=$?
        
        if [ $EXTRACT_EXIT_CODE -eq 0 ]; then
            echo "  Timing data extraction completed for period ${CLK_PERIOD} ps!"
        else
            echo "  Warning: Timing data extraction failed for period ${CLK_PERIOD} ps!"
        fi
    else
        echo ""
        echo "  Error: Synthesis failed for period ${CLK_PERIOD} ps!"
        echo "  Check genus.log for details"
        FAILED_PERIODS+=("${CLK_PERIOD}")
    fi
    
    # Copy genus.log to results directory with prefix before cleanup (if exists)
    if [ -f "genus.log" ]; then
        cp "genus.log" "$RESULTS_DIR/${PREFIX}genus.log.${CLK_PERIOD}"
    fi
    
    # Clean up Genus-generated files
    echo "  Cleaning up Genus-generated files..."
    rm -f genus.cmd* genus_* .genus* .cadence genus.log*
    
    # Remove temporary TCL script
    rm -f "$TEMP_TCL"
    
    echo "  Cleanup complete!"
done

# Summary
echo ""
echo "========================================"
echo "  Summary"
echo "========================================"
echo ""
echo "Total periods processed: $TOTAL_PERIODS"
if [ ${#FAILED_PERIODS[@]} -eq 0 ]; then
    echo "All periods completed successfully!"
else
    echo "Failed periods: ${FAILED_PERIODS[*]}"
    exit 1
fi
echo ""
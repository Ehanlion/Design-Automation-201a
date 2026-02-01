#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Compare Power: Benchmark, 3A, 3B       */
#*                                                  */
#* This script compares power subtotals (leakage,  */
#* internal, switching, total) for benchmark, 3A,  */
#* and 3B, including clock periods and timing      */
#* slack for each configuration.                   */
#*                                                  */
#* Usage: ./scripts/comparePower3A3B.sh            */
#*        or: bash scripts/comparePower3A3B.sh     */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Power Comparison: Benchmark, 3A, 3B"
echo "========================================"
echo ""

# Function to extract power values from power report
extract_power_values() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "N/A N/A N/A N/A"
        return
    fi
    # Look for "Subtotal" line and extract leakage, internal, switching, total
    local line=$(grep -E "^\s+Subtotal" "$file" | head -1)
    if [ -z "$line" ]; then
        echo "N/A N/A N/A N/A"
        return
    fi
    # Extract the 4 values (leakage, internal, switching, total)
    local leakage=$(echo "$line" | awk '{print $2}')
    local internal=$(echo "$line" | awk '{print $3}')
    local switching=$(echo "$line" | awk '{print $4}')
    local total=$(echo "$line" | awk '{print $5}')
    echo "$leakage $internal $switching $total"
}

# Function to extract clock period from timing report (in ps)
extract_clock_period_from_timing() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "N/A"
        return
    fi
    # Look for "Clock Edge:+" line and extract the value
    local period=$(grep -E "^\s+Clock Edge:\+" "$file" | head -1 | awk '{print $3}' | tr -d ' ')
    if [ -z "$period" ]; then
        echo "N/A"
    else
        echo "$period"
    fi
}

# Function to extract clock period from SDC file (in ps)
extract_clock_period_from_sdc() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "N/A"
        return
    fi
    # Look for create_clock line and extract period (in ns, convert to ps)
    local period_ns=$(grep -E "create_clock.*-period" "$file" | head -1 | sed -n 's/.*-period \([0-9.]*\).*/\1/p')
    if [ -z "$period_ns" ]; then
        echo "N/A"
    else
        # Convert ns to ps (multiply by 1000)
        local period_ps=$(awk "BEGIN {printf \"%.0f\", $period_ns * 1000}")
        echo "$period_ps"
    fi
}

# Function to extract slack from timing report (in ps)
extract_slack_from_timing() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "N/A"
        return
    fi
    # Look for "Slack:=" line and extract the value
    local slack=$(grep -E "^\s+Slack:=" "$file" | head -1 | awk '{print $2}' | tr -d ' ')
    if [ -z "$slack" ]; then
        echo "N/A"
    else
        echo "$slack"
    fi
}

# Function to extract clock gating cell count from gates report
extract_clock_gating_count() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "0"
        return
    fi
    # Look for "clock_gating_integrated_cell" line and extract instances count
    local count=$(grep -E "^\s*clock_gating_integrated_cell" "$file" | awk '{print $2}')
    if [ -z "$count" ]; then
        echo "0"
    else
        echo "$count"
    fi
}

# Function to extract total area from gates report
extract_total_area() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo "N/A"
        return
    fi
    # Look for the last "total" line (the summary total at the bottom)
    local area=$(grep -E "^\s*total\s+" "$file" | tail -1 | awk '{print $3}')
    if [ -z "$area" ]; then
        echo "N/A"
    else
        echo "$area"
    fi
}

# Extract data for each configuration
echo "Extracting power and timing data..."
echo ""

# Benchmark
BENCHMARK_POWER="$LAB2_DIR/_benchmark/synth_report_power.txt"
BENCHMARK_TIMING="$LAB2_DIR/_benchmark/synth_report_timing.txt"
BENCHMARK_SDC="$LAB2_DIR/_benchmark/s15850.sdc"
BENCHMARK_GATES="$LAB2_DIR/_benchmark/synth_report_gates.txt"
BENCHMARK_POWER_VALS=$(extract_power_values "$BENCHMARK_POWER")
BENCHMARK_LEAKAGE=$(echo $BENCHMARK_POWER_VALS | awk '{print $1}')
BENCHMARK_INTERNAL=$(echo $BENCHMARK_POWER_VALS | awk '{print $2}')
BENCHMARK_SWITCHING=$(echo $BENCHMARK_POWER_VALS | awk '{print $3}')
BENCHMARK_TOTAL=$(echo $BENCHMARK_POWER_VALS | awk '{print $4}')
BENCHMARK_PERIOD=$(extract_clock_period_from_timing "$BENCHMARK_TIMING")
if [ "$BENCHMARK_PERIOD" = "N/A" ]; then
    BENCHMARK_PERIOD=$(extract_clock_period_from_sdc "$BENCHMARK_SDC")
fi
BENCHMARK_SLACK=$(extract_slack_from_timing "$BENCHMARK_TIMING")
BENCHMARK_CLK_GATING=$(extract_clock_gating_count "$BENCHMARK_GATES")
BENCHMARK_AREA=$(extract_total_area "$BENCHMARK_GATES")

# 3A
POWER_3A="$LAB2_DIR/results/3A_synth_report_power.txt"
TIMING_3A="$LAB2_DIR/results/3A_synth_report_timing.txt"
SDC_3A="$LAB2_DIR/results/3A_s15850.sdc"
GATES_3A="$LAB2_DIR/results/3A_synth_report_gates.txt"
POWER_VALS_3A=$(extract_power_values "$POWER_3A")
LEAKAGE_3A=$(echo $POWER_VALS_3A | awk '{print $1}')
INTERNAL_3A=$(echo $POWER_VALS_3A | awk '{print $2}')
SWITCHING_3A=$(echo $POWER_VALS_3A | awk '{print $3}')
TOTAL_3A=$(echo $POWER_VALS_3A | awk '{print $4}')
PERIOD_3A=$(extract_clock_period_from_timing "$TIMING_3A")
if [ "$PERIOD_3A" = "N/A" ]; then
    PERIOD_3A=$(extract_clock_period_from_sdc "$SDC_3A")
fi
SLACK_3A=$(extract_slack_from_timing "$TIMING_3A")
CLK_GATING_3A=$(extract_clock_gating_count "$GATES_3A")
AREA_3A=$(extract_total_area "$GATES_3A")

# 3B
POWER_3B="$LAB2_DIR/results/3B_synth_report_power.txt"
TIMING_3B="$LAB2_DIR/results/3B_synth_report_timing.txt"
SDC_3B="$LAB2_DIR/results/3B_s15850.sdc"
GATES_3B="$LAB2_DIR/results/3B_synth_report_gates.txt"
POWER_VALS_3B=$(extract_power_values "$POWER_3B")
LEAKAGE_3B=$(echo $POWER_VALS_3B | awk '{print $1}')
INTERNAL_3B=$(echo $POWER_VALS_3B | awk '{print $2}')
SWITCHING_3B=$(echo $POWER_VALS_3B | awk '{print $3}')
TOTAL_3B=$(echo $POWER_VALS_3B | awk '{print $4}')
PERIOD_3B=$(extract_clock_period_from_timing "$TIMING_3B")
if [ "$PERIOD_3B" = "N/A" ]; then
    PERIOD_3B=$(extract_clock_period_from_sdc "$SDC_3B")
fi
SLACK_3B=$(extract_slack_from_timing "$TIMING_3B")
CLK_GATING_3B=$(extract_clock_gating_count "$GATES_3B")
AREA_3B=$(extract_total_area "$GATES_3B")

# Print summary header
echo "Area and Clock Gating Summary"
echo "=================================================================================="
printf "%-12s %20s %20s\n" \
    "Config" "Clock Gating Cells" "Total Area"
echo "=================================================================================="

# Print data for each configuration
printf "%-12s %20s %20s\n" \
    "Benchmark" "$BENCHMARK_CLK_GATING" "$BENCHMARK_AREA"

printf "%-12s %20s %20s\n" \
    "Problem 3A" "$CLK_GATING_3A" "$AREA_3A"

printf "%-12s %20s %20s\n" \
    "Problem 3B" "$CLK_GATING_3B" "$AREA_3B"

echo "=================================================================================="
echo ""

echo "Power Comparison Summary"
echo "=========================================================================================================="
printf "%-12s %12s %12s %15s %15s %15s %15s %12s\n" \
    "Config" "Period (ps)" "Slack (ps)" "Leakage (W)" "Internal (W)" "Switching (W)" "Total (W)" ""
echo "=========================================================================================================="

# Print data for each configuration
printf "%-12s %12s %12s %15s %15s %15s %15s\n" \
    "Benchmark" "$BENCHMARK_PERIOD" "$BENCHMARK_SLACK" \
    "$BENCHMARK_LEAKAGE" "$BENCHMARK_INTERNAL" "$BENCHMARK_SWITCHING" "$BENCHMARK_TOTAL"

printf "%-12s %12s %12s %15s %15s %15s %15s\n" \
    "Problem 3A" "$PERIOD_3A" "$SLACK_3A" \
    "$LEAKAGE_3A" "$INTERNAL_3A" "$SWITCHING_3A" "$TOTAL_3A"

printf "%-12s %12s %12s %15s %15s %15s %15s\n" \
    "Problem 3B" "$PERIOD_3B" "$SLACK_3B" \
    "$LEAKAGE_3B" "$INTERNAL_3B" "$SWITCHING_3B" "$TOTAL_3B"

echo "=========================================================================================================="
echo ""

# Check if any values are missing
MISSING_VALS=0
if [ "$BENCHMARK_LEAKAGE" = "N/A" ] || [ "$BENCHMARK_INTERNAL" = "N/A" ] || \
   [ "$BENCHMARK_SWITCHING" = "N/A" ] || [ "$BENCHMARK_TOTAL" = "N/A" ] || \
   [ "$BENCHMARK_PERIOD" = "N/A" ] || [ "$BENCHMARK_SLACK" = "N/A" ]; then
    MISSING_VALS=1
fi

if [ "$LEAKAGE_3A" = "N/A" ] || [ "$INTERNAL_3A" = "N/A" ] || \
   [ "$SWITCHING_3A" = "N/A" ] || [ "$TOTAL_3A" = "N/A" ] || \
   [ "$PERIOD_3A" = "N/A" ] || [ "$SLACK_3A" = "N/A" ]; then
    MISSING_VALS=1
fi

if [ "$LEAKAGE_3B" = "N/A" ] || [ "$INTERNAL_3B" = "N/A" ] || \
   [ "$SWITCHING_3B" = "N/A" ] || [ "$TOTAL_3B" = "N/A" ] || \
   [ "$PERIOD_3B" = "N/A" ] || [ "$SLACK_3B" = "N/A" ]; then
    MISSING_VALS=1
fi

if [ $MISSING_VALS -eq 1 ]; then
    echo "Warning: Some values could not be extracted. Check that all report files exist."
    echo ""
    echo "Expected files:"
    echo "  Benchmark:"
    echo "    - $BENCHMARK_POWER"
    echo "    - $BENCHMARK_TIMING or $BENCHMARK_SDC"
    echo "    - $BENCHMARK_GATES"
    echo "  Problem 3A:"
    echo "    - $POWER_3A"
    echo "    - $TIMING_3A or $SDC_3A"
    echo "    - $GATES_3A"
    echo "  Problem 3B:"
    echo "    - $POWER_3B"
    echo "    - $TIMING_3B or $SDC_3B"
    echo "    - $GATES_3B"
    exit 1
fi

echo "Summary complete!"
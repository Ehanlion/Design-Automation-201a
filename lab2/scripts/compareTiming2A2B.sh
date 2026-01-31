#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Compare Timing: 2A, 2B, and Benchmark  */
#*                                                  */
#* This script compares timing slack and clock     */
#* period for Problem 2A, 2B, and the benchmark   */
#*                                                  */
#* Usage: ./scripts/compareTiming2A2B.sh           */
#*        or: bash scripts/compareTiming2A2B.sh    */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

echo "========================================"
echo "  Timing Comparison: 2A, 2B, Benchmark"
echo "========================================"
echo ""

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

# Extract data for each configuration
echo "Extracting timing data..."
echo ""

# Benchmark
BENCHMARK_TIMING="$LAB2_DIR/_benchmark/synth_report_timing.txt"
BENCHMARK_SDC="$LAB2_DIR/_benchmark/s15850.sdc"
BENCHMARK_PERIOD=$(extract_clock_period_from_timing "$BENCHMARK_TIMING")
if [ "$BENCHMARK_PERIOD" = "N/A" ]; then
    BENCHMARK_PERIOD=$(extract_clock_period_from_sdc "$BENCHMARK_SDC")
fi
BENCHMARK_SLACK=$(extract_slack_from_timing "$BENCHMARK_TIMING")

# 2A
TIMING_2A="$LAB2_DIR/results/2A_synth_report_timing.txt"
SDC_2A="$LAB2_DIR/results/2A_s15850.sdc"
PERIOD_2A=$(extract_clock_period_from_timing "$TIMING_2A")
if [ "$PERIOD_2A" = "N/A" ]; then
    PERIOD_2A=$(extract_clock_period_from_sdc "$SDC_2A")
fi
SLACK_2A=$(extract_slack_from_timing "$TIMING_2A")

# 2B
TIMING_2B="$LAB2_DIR/results/2B_synth_report_timing.txt"
SDC_2B="$LAB2_DIR/results/2B_s15850.sdc"
PERIOD_2B=$(extract_clock_period_from_timing "$TIMING_2B")
if [ "$PERIOD_2B" = "N/A" ]; then
    PERIOD_2B=$(extract_clock_period_from_sdc "$SDC_2B")
fi
SLACK_2B=$(extract_slack_from_timing "$TIMING_2B")

# Print summary
printf "%-15s %15s %15s\n" "Configuration" "Clock Period (ps)" "Timing Slack (ps)"
echo "---------------------------------------------------------------"
printf "%-15s %15s %15s\n" "Benchmark" "$BENCHMARK_PERIOD" "$BENCHMARK_SLACK"
printf "%-15s %15s %15s\n" "Problem 2A" "$PERIOD_2A" "$SLACK_2A"
printf "%-15s %15s %15s\n" "Problem 2B" "$PERIOD_2B" "$SLACK_2B"
echo "---------------------------------------------------------------"
echo ""

# Check if any values are missing
if [ "$BENCHMARK_PERIOD" = "N/A" ] || [ "$BENCHMARK_SLACK" = "N/A" ] || \
   [ "$PERIOD_2A" = "N/A" ] || [ "$SLACK_2A" = "N/A" ] || \
   [ "$PERIOD_2B" = "N/A" ] || [ "$SLACK_2B" = "N/A" ]; then
    echo "Warning: Some values could not be extracted. Check that all report files exist."
    echo ""
    echo "Expected files:"
    echo "  - $BENCHMARK_TIMING or $BENCHMARK_SDC"
    echo "  - $TIMING_2A or $SDC_2A"
    echo "  - $TIMING_2B or $SDC_2B"
    exit 1
fi

echo "Summary complete!"
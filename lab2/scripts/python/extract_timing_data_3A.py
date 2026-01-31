#!/usr/bin/env python3
"""
Extract clock period, total area, instances, and slack from 3A files and append to CSV.

Extracts:
- Period from lab2_3A.tcl (set clk_period value)
- Total area and instances from results/3A_synth_report_gates.txt
- Slack from results/3A_synth_report_timing.txt

Usage: python3 scripts/python/extract_timing_data_3A.py
       or: bash scripts/extractTimingData3A.sh
"""

import re
import os
from pathlib import Path

# Get the directory where this script is located
SCRIPT_DIR = Path(__file__).parent.absolute()
# Get the scripts directory (parent of python)
SCRIPTS_DIR = SCRIPT_DIR.parent
# Get the lab2 root directory (parent of scripts)
LAB2_DIR = SCRIPTS_DIR.parent

# Paths
TCL_FILE = LAB2_DIR / "lab2_3A.tcl"
GATES_REPORT = LAB2_DIR / "results" / "3A_synth_report_gates.txt"
TIMING_REPORT = LAB2_DIR / "results" / "3A_synth_report_timing.txt"
CSV_OUTPUT = LAB2_DIR / "results" / "3A_plotting_data.csv"

def extract_period_from_tcl(tcl_file):
    """Extract clock period from TCL file."""
    period = None
    
    with open(tcl_file, 'r') as f:
        for line in f:
            # Look for: "set clk_period 600"
            match = re.search(r'set\s+clk_period\s+(\d+)', line)
            if match:
                period = int(match.group(1))
                break
    
    return period

def extract_area_and_instances(gates_file):
    """Extract total area and instances from gates report."""
    area = None
    instances = None
    
    with open(gates_file, 'r') as f:
        for line in f:
            # Look for: "total            423  1005.480"
            if line.strip().startswith('total'):
                # Extract numbers from the line
                match = re.search(r'total\s+(\d+)\s+([\d.]+)', line)
                if match:
                    instances = int(match.group(1))
                    area = float(match.group(2))
                    break
    
    return area, instances

def extract_slack(timing_file):
    """Extract slack from timing report."""
    slack = None
    
    with open(timing_file, 'r') as f:
        for line in f:
            # Look for slack: "Slack:=      27"
            if 'Slack:=' in line:
                # Extract the number after "Slack:="
                match = re.search(r'Slack:=\s+(-?\d+)', line)
                if match:
                    slack = int(match.group(1))
                    # Usually the first slack found is the worst-case, which is what we want
                    break
    
    return slack

def append_to_csv(csv_file, period, area, instances, slack):
    """Append period, area, instances, and slack to CSV file, then sort by period."""
    # Create directory if it doesn't exist
    csv_file.parent.mkdir(parents=True, exist_ok=True)
    
    # Read existing data if file exists
    rows = []
    file_exists = csv_file.exists()
    
    if file_exists:
        with open(csv_file, 'r') as f:
            lines = f.readlines()
            # Skip header if it exists
            if lines and lines[0].strip().startswith('period'):
                rows = [line.strip() for line in lines[1:] if line.strip()]
            else:
                rows = [line.strip() for line in lines if line.strip()]
    
    # Add new row
    new_row = f"{period},{area},{instances},{slack}"
    rows.append(new_row)
    
    # Sort rows by period (first column)
    def get_period(row):
        try:
            return int(row.split(',')[0])
        except (ValueError, IndexError):
            return float('inf')  # Put invalid rows at the end
    
    rows.sort(key=get_period)
    
    # Write back to file with header
    with open(csv_file, 'w') as f:
        f.write("period,total area,instances,slack\n")
        for row in rows:
            f.write(f"{row}\n")

def main():
    import sys
    
    errors = []
    
    # Check if period was provided as command-line argument
    period = None
    if len(sys.argv) > 1:
        try:
            period = int(sys.argv[1])
            print(f"Using period from command-line argument: {period} ps")
        except ValueError:
            print(f"Warning: Invalid period argument '{sys.argv[1]}', will try to extract from TCL file")
    
    # If period not provided, extract from TCL file
    if period is None:
        # Check if TCL file exists
        if not TCL_FILE.exists():
            errors.append(f"TCL file not found at {TCL_FILE}")
        
        if errors:
            for error in errors:
                print(f"Error: {error}")
            return 1
        
        # Extract period from TCL file
        period = extract_period_from_tcl(TCL_FILE)
        if period is None:
            print("Error: Could not extract clock period from TCL file")
            return 1
    
    # Check if gates report exists
    if not GATES_REPORT.exists():
        errors.append(f"Gates report not found at {GATES_REPORT}")
    
    # Check if timing report exists
    if not TIMING_REPORT.exists():
        errors.append(f"Timing report not found at {TIMING_REPORT}")
    
    if errors:
        for error in errors:
            print(f"Error: {error}")
        return 1
    
    # Extract area and instances from gates report
    area, instances = extract_area_and_instances(GATES_REPORT)
    if area is None or instances is None:
        print("Error: Could not extract total area and instances from gates report")
        return 1
    
    # Extract slack from timing report
    slack = extract_slack(TIMING_REPORT)
    if slack is None:
        print("Error: Could not extract slack from timing report")
        return 1
    
    # Append to CSV
    append_to_csv(CSV_OUTPUT, period, area, instances, slack)
    
    print(f"Successfully extracted timing data:")
    print(f"  Period: {period} ps")
    print(f"  Total Area: {area}")
    print(f"  Instances: {instances}")
    print(f"  Slack: {slack} ps")
    print(f"  Appended to: {CSV_OUTPUT}")
    
    return 0

if __name__ == "__main__":
    exit(main())

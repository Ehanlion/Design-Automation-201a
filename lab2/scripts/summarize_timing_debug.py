#!/usr/bin/env python3
"""
UCLA EE 201A -- VLSI Design Automation
Lab 2 - Timing Debug Report Summarizer

This script parses the debug timing report and generates a summary
showing path numbers, MET/VIOLATED status, slack values, and endpoints.
"""

import sys
import re
import os

def parse_timing_debug(file_path):
    """Parse the debug timing file and extract path information."""
    paths = []
    
    if not os.path.exists(file_path):
        print(f"Error: File '{file_path}' not found!", file=sys.stderr)
        return None
    
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Pattern to match path headers: "Path N: STATUS (slack) Setup Check..."
    path_pattern = re.compile(
        r'Path\s+(\d+):\s+(MET|VIOLATED)\s+\(([-\d]+)\s+ps\)\s+Setup\s+Check\s+with\s+Pin\s+([^\n]+)'
    )
    
    # Pattern to match startpoint and endpoint
    startpoint_pattern = re.compile(r'Startpoint:\s+\([RF]\)\s+([^\n]+)')
    endpoint_pattern = re.compile(r'Endpoint:\s+\([RF]\)\s+([^\n]+)')
    
    # Find all path matches
    for match in path_pattern.finditer(content):
        path_num = int(match.group(1))
        status = match.group(2)
        slack_ps = int(match.group(3))
        pin = match.group(4).strip()
        
        # Find startpoint and endpoint for this path
        # Get the section starting from this match
        start_pos = match.start()
        # Find the next path or end of file
        next_match = path_pattern.search(content, start_pos + 1)
        end_pos = next_match.start() if next_match else len(content)
        
        section = content[start_pos:end_pos]
        
        startpoint_match = startpoint_pattern.search(section)
        endpoint_match = endpoint_pattern.search(section)
        
        startpoint = startpoint_match.group(1).strip() if startpoint_match else "N/A"
        endpoint = endpoint_match.group(1).strip() if endpoint_match else "N/A"
        
        paths.append({
            'path': path_num,
            'status': status,
            'slack_ps': slack_ps,
            'startpoint': startpoint,
            'endpoint': endpoint,
            'pin': pin
        })
    
    return paths

def generate_summary(paths, output_file=None):
    """Generate a formatted summary of timing paths."""
    if not paths:
        print("No paths found in timing debug file.", file=sys.stderr)
        return
    
    # Sort by path number
    paths.sort(key=lambda x: x['path'])
    
    # Count MET vs VIOLATED
    met_count = sum(1 for p in paths if p['status'] == 'MET')
    violated_count = sum(1 for p in paths if p['status'] == 'VIOLATED')
    
    # Generate summary text
    summary_lines = []
    summary_lines.append("=" * 80)
    summary_lines.append("  TIMING DEBUG REPORT SUMMARY")
    summary_lines.append("=" * 80)
    summary_lines.append("")
    summary_lines.append(f"Total Paths Analyzed: {len(paths)}")
    summary_lines.append(f"  MET:      {met_count}")
    summary_lines.append(f"  VIOLATED: {violated_count}")
    summary_lines.append("")
    summary_lines.append("-" * 80)
    summary_lines.append(f"{'Path':<6} {'Status':<10} {'Slack (ps)':<12} {'Startpoint':<30} {'Endpoint':<30}")
    summary_lines.append("-" * 80)
    
    for p in paths:
        status_str = p['status']
        slack_str = f"{p['slack_ps']:>6} ps"
        # Truncate long names
        startpoint_short = p['startpoint'][:28] + ".." if len(p['startpoint']) > 30 else p['startpoint']
        endpoint_short = p['endpoint'][:28] + ".." if len(p['endpoint']) > 30 else p['endpoint']
        
        summary_lines.append(f"{p['path']:<6} {status_str:<10} {slack_str:<12} {startpoint_short:<30} {endpoint_short:<30}")
    
    summary_lines.append("-" * 80)
    summary_lines.append("")
    
    summary_text = "\n".join(summary_lines)
    
    # Output to file or stdout
    if output_file:
        with open(output_file, 'w') as f:
            f.write(summary_text)
        print(f"Summary written to: {output_file}")
    else:
        print(summary_text)

def main():
    if len(sys.argv) < 2:
        print("Usage: summarize_timing_debug.py <debug_timing_file> [output_file]", file=sys.stderr)
        print("  debug_timing_file: Path to the debug timing report file", file=sys.stderr)
        print("  output_file:       Optional output file (default: stdout)", file=sys.stderr)
        sys.exit(1)
    
    debug_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    paths = parse_timing_debug(debug_file)
    if paths is None:
        sys.exit(1)
    
    generate_summary(paths, output_file)

if __name__ == "__main__":
    main()

#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Clean Script                            */
#*                                                  */
#* Cleans up all generated files from Lab 2        */
#*                                                  */
#* Usage: ./clean.sh                               */
#*        or: bash clean.sh                        */
#*                                                  */
#* Works from scripts/ directory or lab2/ root     */
#**************************************************/

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Change to the lab2 directory (parent of scripts/)
LAB2_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB2_DIR" || exit 1

echo "Cleaning Lab 2 generated files..."
echo "Working directory: $(pwd)"

# Remove Genus synthesis output directories
rm -rf fv

# Remove Genus log files
rm -f *.log
rm -f *.log.gz
rm -f *.log.*

# Remove command files
rm -f *.cmd
rm -f *.cmd.gz

# Remove object files
rm -f *.o

# Remove other Genus-generated files
rm -rf .cadence
rm -rf genus_*
rm -f .genus*
rm -f genus.log*
rm -f genus.cmd*

# Remove backup files
rm -f *~
rm -f *.bak

# Remove any .syn files
rm -f *.syn

# Remove timing and power files
rm -f *.rpt
rm -f *.timing
rm -f *.power

# Remove synthesized netlists
rm -f *_synth.v

# Remove SDC files
rm -f *.sdc

# Remove synthesis report files
rm -f synth_report_timing.txt
rm -f synth_report_gates.txt
rm -f synth_report_power.txt

# Remove problem1 results
rm -f problem1_results.txt

echo "Cleanup complete!"

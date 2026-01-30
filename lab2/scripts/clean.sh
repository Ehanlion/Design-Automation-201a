#!/bin/bash
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Clean Script                            */
#*                                                  */
#* Cleans up all generated files from Lab 2        */
#*                                                  */
#* Usage: ./clean.sh                               */
#*        or: bash clean.sh                        */
#**************************************************/

echo "Cleaning Lab 2 generated files..."

# Remove Genus synthesis output directories
rm -rf fv
rm -rf results

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

# Remove synthesized netlists (if in root directory)
rm -f *_synth.v

# Remove SDC files (if generated in root directory)
rm -f *.sdc

# Note: results/ directory contains all synthesis outputs and will be removed above

echo "Cleanup complete!"

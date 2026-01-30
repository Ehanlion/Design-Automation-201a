# UCLA EE 201A -- VLSI Design Automation
# Winter 2021
# Lab 2 Problem 2: Clock Period Optimization

#**************************************************/
#* Script for Cadence Genus synthesis             */
#*                                                */
#* To run: genus < lab2_problem2.tcl              */
#* Or open genus by running: genus                */
#* Then run: source lab2_problem2.tcl             */
#*                                                */
#* Or use: bash scripts/run_problem2.sh          */
#*                                                */
#**************************************************/

# Use verbose console output
set_db information_level 9

# Suppress warnings that are OK to ignore. Excluding some Liberty import Warnings
suppress_messages LBR-40 LBR-362 LBR-9

# Set path where Genus will look for RTL design files
set_db init_hdl_search_path {./} 

# Make sure errors are reported for missing modules
set_db hdl_error_on_blackbox true

# Set path where Genus will look for timing libraries, i.e., Liberty files (.lib suffix)
set_db init_lib_search_path {./}

# Set filename of the Liberty file for our Nangate library
set_db library NangateOpenCellLibrary_typical.lib

# Set list of all necessary RTL files written in HDLs like Verilog or VHDL, separated by spaces
set hdl_files {s15850.v}

# Set the name of the design within Genus
set DNAME s15850

# Set the name of the top-level module used in the design HDL
set DESIGN s15850_bench

# Set the name of the clock signal used in the design HDL
set clkpin blif_clk_net

# Load Verilog design files into Genus
read_hdl -v2001 ${hdl_files}

# Initialize design from design RTL
elaborate $DESIGN

#**************************************************/

# Apply design constraints for logic synthesis -- define clock period, slew rate, relative block I/O delays, etc. Here, we've only set timing constraints, with no area or power constraints listed.

# Set clock period
# EE 201A Lab 2 Problem 2: Modify the clock period constraint to find the best achievable frequency
# Start with a reasonable value and iteratively reduce until slack ≈ 0

# Carry over the best clock period from Problem 2A
set clk_period 460

set clock [define_clock -period ${clk_period} -name ${clkpin} [clock_ports]]

# Set block I/O external delays (needed for tool to "close timing") (Suppressing a Warning)
suppress_messages TUI-253
set_input_delay -clock ${clkpin} 0 [vfind /designs/${DESIGN}/ports -port *]
set_output_delay -clock ${clkpin} 0 [vfind /designs/${DESIGN}/ports -port *]

# Set clock slew rate (rise/fall time)
dc::set_clock_transition .1 ${clkpin}

#**************************************************/

# Check for any issues
check_design -unresolved

# List possible timing problems prior to synthesis
report_timing -lint

# Synthesize design and map it to technology library

# OLD SYNTHESIS COMMANDS
# Replace these with new synthesis commands
# syn_generic
# syn_map
# syn_opt

# New synthesis commands
# 1. Allow flattening of hierarchy
# Removes module boundaries, allowing the tool to optimize logic across blocks.
set_db auto_ungroup both

# 2. Synthesize to generic gates with High Effort
# Tells the tool to explore more architectural variations.
syn_generic -effort high

# 3. Map to technology library with High Effort
syn_map -effort high

# 4. Optimizaton with Retiming and High Effort
# -retime: Moves registers through logic to balance path delays (critical for speed).
# -effort high: Spend more CPU time finding the best solution.
# removed retime option because of errors
syn_opt -effort high -incremental

# 5. Incremental Optimization (The "TNS" Hint)
# Once WNS (Worst Negative Slack) is stuck, this pass helps reduce 
# Total Negative Slack (TNS) on other paths.
# syn_opt -incremental

# List possible timing problems after synthesis
report_timing -lint

# Generate post-synthesis reports on timing, area, and power estimates
report_timing > synth_report_timing.txt
report_gates  > synth_report_gates.txt
report_power  > synth_report_power.txt

# Output the synthesized netlist to a new Verilog file
write_hdl > ${DNAME}_synth.v

# Export the design constraints to SDC file
write_sdc >  ${DNAME}.sdc

# Report final timing again to console for user to review
report_timing -lint -verbose

puts \n 
puts "Synthesis Finished!         "
puts \n
puts "Check current directory for synthesis results and reports."
puts \n
 
# Exit Genus 
# quit

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

# For clock period, automatically copy value pairs for plotting after iterations
# No History tracking here
set clk_period 360

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

# ============================================================
# OPTIMIZATION SETTINGS FOR TIMING IMPROVEMENT
# Copied optimizations from 2B
# Don't change, just increase the period until slack is 200ps
# ============================================================

# Allow Genus to dissolve hierarchy boundaries to merge logic
# Optimize all negative slack endpoints.
set_db auto_ungroup both
set_db tns_opto true

# Set effort levels to high for all stages of synthesis
set_db syn_generic_effort high
set_db syn_map_effort high
set_db syn_opt_effort high
set_db retime_effort high

# Standard synthesis from prior skeleton example
syn_generic
syn_map

# Define a new cost group for timing optimization, give a higher weight, and tell synthesis to focus on it.
define_cost_group -name critical_path -weight 10 -design [get_designs *]

# assign paths relating to the clock signal (critical) to the cost group
path_group -from [all_registers -clock ${clkpin}] -to [all_registers -clock ${clkpin}] -group critical_path -name clock_paths

# Standard simulation
syn_opt
retime -min_delay
syn_opt -incremental

# ============================================================
# END OF OPTIMIZATION SETTINGS FOR TIMING IMPROVEMENT
# ============================================================

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

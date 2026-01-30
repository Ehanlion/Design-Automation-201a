#!/usr/bin/tclsh
#**************************************************/
#* UCLA EE 201A -- VLSI Design Automation          */
#* Lab 2 - Problem 1: TCL Practice Problem         */
#*                                                  */
#* This script reads wirelength data from a file   */
#* and calculates:                                  */
#*   A. Average wirelength                          */
#*   B. Number of wires with length > 50            */
#*                                                  */
#* To run: tclsh lab2_problem1.tcl                  */
#*         or: ./lab2_problem1.tcl                  */
#**************************************************/

# Set the input file path
set input_file "results/wirelength.txt"

# Check if file exists
if {![file exists $input_file]} {
    puts "Error: File '$input_file' not found!"
    puts "Please ensure wirelength.txt is in the results/ directory."
    exit 1
}

# Open and read the wirelength file
set fp [open $input_file r]
set wirelengths [list]

# Read each line and store in list
while {[gets $fp line] >= 0} {
    # Skip empty lines
    if {[string trim $line] ne ""} {
        lappend wirelengths [string trim $line]
    }
}
close $fp

# Get total count
set count [llength $wirelengths]

# Check if we have data
if {$count == 0} {
    puts "Error: No wirelength data found in file!"
    exit 1
}

# Calculate average wirelength
set sum 0.0
foreach wl $wirelengths {
    set sum [expr {$sum + $wl}]
}
set avg [expr {$sum / $count}]

# Count wires with length > 50
set count_over_50 0
foreach wl $wirelengths {
    if {$wl > 50} {
        incr count_over_50
    }
}

# Print results
puts ""
puts "=================================================="
puts "  Lab 2 - Problem 1: Wirelength Analysis Results"
puts "=================================================="
puts ""
puts "Total number of wires: $count"
puts ""
puts "A. Average wirelength: $avg"
puts ""
puts "B. Number of wires with length > 50: $count_over_50"
puts ""
puts "=================================================="
puts ""

# Optional: Write results to a file
set output_file "results/problem1_results.txt"
set out_fp [open $output_file w]
puts $out_fp "Lab 2 - Problem 1 Results"
puts $out_fp "========================="
puts $out_fp ""
puts $out_fp "Total wires: $count"
puts $out_fp "Average wirelength: $avg"
puts $out_fp "Wires with length > 50: $count_over_50"
close $out_fp

puts "Results also saved to: $output_file"
puts ""

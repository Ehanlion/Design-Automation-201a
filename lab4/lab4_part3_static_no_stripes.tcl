# UCLA EE 201A -- VLSI Design Automation
# Winter 2026 -- Lab 4 Part 3 (Static Baseline)
#
# @purpose: Provide a fixed no-stripe reference run for Part 3 comparisons.
# @logic: Set fixed environment overrides, then source the shared parameterized flow.
# why: Keeps one implementation of the flow while enabling reproducible static runs.

if {![info exists ::env(LAB4_UTIL)]} {
    set ::env(LAB4_UTIL) 0.918
}
if {![info exists ::env(LAB4_ENABLE_STRIPES)]} {
    set ::env(LAB4_ENABLE_STRIPES) 0
}
if {![info exists ::env(LAB4_OUTPUTDIR)]} {
    set ::env(LAB4_OUTPUTDIR) results/part3_static_no_stripes
}

source ./lab4_part3_param_testbench.tcl

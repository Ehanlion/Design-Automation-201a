# UCLA EE 201A -- VLSI Design Automation
# Winter 2026 -- Lab 4 Part 2 (Timing-Driven)
# Run:  cd lab4 && innovus -batch -no_gui -init lab4_part2.tcl
#
# Same physical design flow as Part 1, with timing-driven placement
# ENABLED and setup/hold/power reports captured at every major step
# for the Problem 2 comparison.

set netlist  "s1494_synth.v"
set top_cell "s1494_bench"
set sdc      "s1494.sdc"
set lef      "NangateOpenCellLibrary.lef"
set DNAME    s1494
set UTIL     0.991

set RESULTSDIR "results/timing_driven"
file mkdir $RESULTSDIR

puts "============================================"
puts "  Lab 4 Part 2 -- Timing-Driven"
puts "  Utilization:  $UTIL"
puts "  Output dir:   $RESULTSDIR"
puts "============================================"

# ---- Initialize design (same as Part 1) ----
suppressMessage TECHLIB-436
suppressMessage IMPVL-159
set init_verilog $netlist
set init_design_netlisttype "Verilog"
set init_design_settop 1
set init_top_cell $top_cell
set init_lef_file $lef
set init_pwr_net VDD
set init_gnd_net VSS
source ./rc.mmode.tcl
init_design -setup _default_view_ -hold _default_view_
setAnalysisMode -analysisType onChipVariation -cppr both
setDesignMode -process 45

# DON'T CHANGE: Limit number of metal/routing layers
setMaxRouteLayer 4

floorplan -r 1.0 $UTIL 6 6 6 6

globalNetConnect VDD -type pgpin -pin VDD -inst * -module {}
globalNetConnect VSS -type pgpin -pin VSS -inst * -module {}

# DON'T CHANGE addRing statement.
addRing -layer {top metal1 bottom metal1 left metal2 right metal2} \
        -spacing {top 1 bottom 1 left 1 right 1} \
        -width {top 1 bottom 1 left 1 right 1} \
        -center 1 \
        -nets { VDD VSS }

# PART 2 OUTPUT: checkpoint before placement
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/01_before_placement_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/01_before_placement_hold.tarpt
report_power > ${RESULTSDIR}/01_before_placement_power.rpt

# ---- Placement ----
# CHANGED vs Part 1: explicitly enable timing-driven placement for the comparison
setPlaceMode -place_global_place_io_pins true -reorderScan false
setPlaceMode -timingDriven true
placeDesign
refinePlace

# PART 2 OUTPUT: checkpoint after placement
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/02_after_placement_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/02_after_placement_hold.tarpt
report_power > ${RESULTSDIR}/02_after_placement_power.rpt

# ---- Pre-CTS optimization ----
optDesign -preCTS

# PART 2 OUTPUT: checkpoint after optDesign -preCTS
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/03_after_opt_prects_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/03_after_opt_prects_hold.tarpt
report_power > ${RESULTSDIR}/03_after_opt_prects_power.rpt

# ---- Clock tree synthesis (same as Part 1) ----
sroute -connect { corePin } -corePinTarget { firstAfterRowEnd } -nets { VDD VSS }
trialroute
buildTimingGraph

set_ccopt_property buffer_cells {BUF_X1 BUF_X2}
set_ccopt_property inverter_cells {INV_X1 INV_X2 INV_X4 INV_X8 INV_X16}
create_ccopt_clock_tree_spec
ccopt_design -cts

refinePlace
setTrialRouteMode -highEffort true
trialRoute

# ---- Post-CTS RC extraction (same as Part 1) ----
setExtractRCMode -layerIndependent 1
extractRC

# PART 2 OUTPUT: checkpoint after post-CTS RC extraction
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/05_after_postcts_rc_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/05_after_postcts_rc_hold.tarpt
report_power > ${RESULTSDIR}/05_after_postcts_rc_power.rpt

# ---- Post-CTS hold optimization (same as Part 1) ----
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
optDesign -postCTS -hold

# PART 2 OUTPUT: checkpoint after optDesign -postCTS -hold
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/06_after_opt_postcts_hold_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/06_after_opt_postcts_hold_hold.tarpt
report_power > ${RESULTSDIR}/06_after_opt_postcts_hold_power.rpt

# ---- Pre-route RC extraction (same as Part 1) ----
setExtractRCMode -engine preRoute -assumeMetFill
extractRC
buildTimingGraph

# PART 2 OUTPUT: checkpoint after pre-route RC extraction
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/07_after_preroute_rc_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/07_after_preroute_rc_hold.tarpt
report_power > ${RESULTSDIR}/07_after_preroute_rc_power.rpt

# ---- Reconnect tie cells before routing (same as Part 1) ----
globalNetConnect VDD -type tiehi
globalNetConnect VDD -type pgpin -pin VDD -override
globalNetConnect VSS -type tielo
globalNetConnect VSS -type pgpin -pin VSS -override

# ---- Routing (match Part 1 logical flow) ----
# Match Part 1 by using the combined global+detail routing step.
globalDetailRoute

# PART 2 OUTPUT: checkpoint after global route
# Part 1 performs routing via globalDetailRoute, so this checkpoint reflects
# the combined route step while preserving the required output file ordering.
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/08_after_global_route_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/08_after_global_route_hold.tarpt
report_power > ${RESULTSDIR}/08_after_global_route_power.rpt

# PART 2 OUTPUT: checkpoint after detail route
# Preserve the Problem 2 output checkpoints/files in order.
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/09_after_detail_route_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/09_after_detail_route_hold.tarpt
report_power > ${RESULTSDIR}/09_after_detail_route_power.rpt

# ---- Post-route optimization (same as Part 1) ----
optDesign -postRoute

# PART 2 OUTPUT: checkpoint after optDesign -postRoute (setup)
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/10_after_opt_postroute_setup_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/10_after_opt_postroute_setup_hold.tarpt
report_power > ${RESULTSDIR}/10_after_opt_postroute_setup_power.rpt

optDesign -postRoute -hold

# PART 2 OUTPUT: checkpoint after optDesign -postRoute -hold
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/11_after_opt_postroute_hold_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/11_after_opt_postroute_hold_hold.tarpt
report_power > ${RESULTSDIR}/11_after_opt_postroute_hold_power.rpt

# ---- Post-route RC extraction (same as Part 1) ----
setExtractRCMode -engine postRoute
extractRC
buildTimingGraph

# PART 2 OUTPUT: final checkpoint after post-route RC extraction
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/12_final_postroute_rc_setup.tarpt
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${RESULTSDIR}/12_final_postroute_rc_hold.tarpt
report_power > ${RESULTSDIR}/12_final_postroute_rc_power.rpt

# ---- Filler cells and save (same as Part 1) ----
addFiller -cell FILLCELL_X1 FILLCELL_X2 FILLCELL_X4 FILLCELL_X8 FILLCELL_X16 FILLCELL_X32

saveDesign ${RESULTSDIR}/${DNAME}_timing_driven.invs

puts "*************************************************************"
puts "* Lab 4 Part 2 (Timing-Driven) finished"
puts "* Design: ${RESULTSDIR}/${DNAME}_timing_driven.invs"
puts "* Reports: ${RESULTSDIR}/01_* through 12_*"
puts "*************************************************************"

exit

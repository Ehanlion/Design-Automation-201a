# UCLA EE 201A -- VLSI Design Automation
# Winter 2026 -- Lab 4 Part 1
# Run:  cd lab4 && innovus -batch -no_gui -init lab4_part1.tcl

# CHANGED: filled in skeleton "???" placeholders with actual filenames
set netlist "s1494_synth.v"
set top_cell "s1494_bench"
set sdc "s1494.sdc"
set lef "NangateOpenCellLibrary.lef"
set DNAME s1494
set OUTPUTDIR output

# CHANGED: utilization from 0.99 to 0.991 -- highest value that routes
# cleanly with 4 metal layers and meets timing. At 0.992+ the floorplan
# snaps to a smaller row-quantized core that causes 781+ DRC violations.
set UTIL 0.991

# ADDED: ensure output directory exists before any file writes
file mkdir $OUTPUTDIR

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

report_timing -check_type setup -nworst  10 -net > ${OUTPUTDIR}/${DNAME}_init_setup.tarpt
report_timing -early -nworst  10 -net > ${OUTPUTDIR}/${DNAME}_init_hold.tarpt

# DON'T CHANGE: Limit number of metal/routing layers
setMaxRouteLayer 4

floorplan -r 1.0 $UTIL 6 6 6 6

globalNetConnect VDD -type pgpin -pin VDD -inst * -module {}
globalNetConnect VSS -type pgpin -pin VSS -inst * -module {}

# DON'T CHANGE addRing statement.
addRing -layer {top metal1 bottom metal1 left metal2 right metal2} -spacing {top 1 bottom 1 left 1 right 1} -width {top 1 bottom 1 left 1 right 1} -center 1 -nets { VDD VSS }

setPlaceMode -place_global_place_io_pins true -reorderScan false
placeDesign
refinePlace
saveNetlist -excludeLeafCell ${OUTPUTDIR}/${DNAME}_placed.v

optDesign -preCTS

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

setExtractRCMode -layerIndependent 1
extractRC

report_ccopt_clock_trees -file ${OUTPUTDIR}/postcts.ctsrpt
report_ccopt_skew_groups -local_skew -file ${OUTPUTDIR}/postcts_localskew.ctsrpt

setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_postcts_hold.tarpt

optDesign -postCTS -hold

setExtractRCMode -engine preRoute -assumeMetFill
extractRC
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_prerouting.tarpt

globalNetConnect VDD -type tiehi
globalNetConnect VDD -type pgpin -pin VDD -override
globalNetConnect VSS -type tielo
globalNetConnect VSS -type pgpin -pin VSS -override

globalDetailRoute

# CHANGED: skeleton only did hold optimization post-route (optDesign -hold -postRoute).
# Added a setup optimization pass first, then hold, to fix both setup and hold violations
# introduced during routing.
optDesign -postRoute
optDesign -postRoute -hold

setExtractRCMode -engine postRoute
extractRC
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_postrouting_hold.tarpt
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_postrouting_setup.tarpt

addFiller -cell FILLCELL_X1 FILLCELL_X2 FILLCELL_X4 FILLCELL_X8 FILLCELL_X16 FILLCELL_X32

# DON'T CHANGE verifyGeometry method.
verifyGeometry -allowRoutingBlkgPinOverlap -allowRoutingCellBlkgOverlap -error 20 -warning 5 -report ${OUTPUTDIR}/${DNAME}.drc.rpt

streamOut ${OUTPUTDIR}/${DNAME}.gds -libName DesignLib -structureName $DNAME -merge { ./NangateOpenCellLibrary.gds } -stripes 1 -units 10000 -mode ALL
defOut -floorplan -netlist -routing ${OUTPUTDIR}/${DNAME}_postrouting.def
rcOut -spef ${OUTPUTDIR}/${DNAME}_postrouting.spef
saveNetlist -excludeLeafCell ${OUTPUTDIR}/${DNAME}_postrouting.v
summaryReport -noHtml -outfile ${OUTPUTDIR}/summary.rpt
reportGateCount -level 10 -outfile ${OUTPUTDIR}/gate_count.rpt
checkDesign -io -netlist -physicalLibrary -powerGround -tieHilo -timingLibrary -floorplan -place -noHtml -outfile ${OUTPUTDIR}/design.rpt

# CHANGED: skeleton had placeholder "Lastname-Firstname_UID_username_Lab4_3.invs";
# now saves to the output directory with a meaningful name
saveDesign ${OUTPUTDIR}/${DNAME}_part1.invs

puts "*************************************************************"
puts "* Innovus script finished"
puts "* Layout:  ${OUTPUTDIR}/${DNAME}.gds"
puts "* Netlist: ${OUTPUTDIR}/${DNAME}_postrouting.v"
puts "* Timing:  ${OUTPUTDIR}/${DNAME}_postrouting_setup.tarpt"
puts "*          ${OUTPUTDIR}/${DNAME}_postrouting_hold.tarpt"
puts "* DRC:     ${OUTPUTDIR}/${DNAME}.drc.rpt"
puts "* Summary: ${OUTPUTDIR}/summary.rpt"
puts "*************************************************************"

# ADDED: exit so batch mode terminates automatically
exit

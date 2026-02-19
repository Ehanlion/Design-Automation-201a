# UCLA EE 201A -- VLSI Design Automation
# Winter 2026 -- Lab 4 Part 3
# Run:  cd lab4/scripts && ./runLab4Part3.sh
#
# Same flow as Part 1 but with power stripes added for Problem 3 comparison.

set netlist "s1494_synth.v"
set top_cell "s1494_bench"
set sdc "s1494.sdc"
set lef "NangateOpenCellLibrary.lef"
set DNAME s1494

# Set the Initial Util Value here
# Iterated over this, found 0.918 to be the max
# Must be an issue with the metal strip here
set UTIL 0.918

set OUTPUTDIR results/part3_final_with_stripes
file mkdir $OUTPUTDIR

# Power stripe params (Problem 3: M2 vertical stripes)
# Setup params according to the lab handout
set STRIPE_LAYER metal2
set STRIPE_DIRECTION vertical
set STRIPE_WIDTH_UM 0.21
set STRIPE_SPACING_UM 4.13
set STRIPE_OFFSET_LEFT_UM 2.0
set STRIPE_SET_DISTANCE_UM 8.26


puts "============================================"
puts "  Lab 4 Part 3  --  util = $UTIL, stripes enabled"
puts "  Output: $OUTPUTDIR"
puts "============================================"

# Set Design Params
suppressMessage TECHLIB-436
suppressMessage IMPVL-159
set init_verilog $netlist
set init_design_netlisttype "Verilog"
set init_design_settop 1
set init_top_cell $top_cell
set init_lef_file $lef
set init_pwr_net VDD
set init_gnd_net VSS

# Source the RC model
source ./rc.mmode.tcl

# Start the design simulation here
init_design -setup _default_view_ -hold _default_view_
setAnalysisMode -analysisType onChipVariation -cppr both
setDesignMode -process 45

report_timing -check_type setup -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_init_setup.tarpt
report_timing -early -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_init_hold.tarpt

setMaxRouteLayer 4

floorplan -r 1.0 $UTIL 6 6 6 6

globalNetConnect VDD -type pgpin -pin VDD -inst * -module {}
globalNetConnect VSS -type pgpin -pin VSS -inst * -module {}

addRing -layer {top metal1 bottom metal1 left metal2 right metal2} -spacing {top 1 bottom 1 left 1 right 1} -width {top 1 bottom 1 left 1 right 1} -center 1 -nets { VDD VSS }

# Add the Power Strip Here
# We will compare the results including this to the results without this from part 1
# Add M2 power stripes (VDD/VSS alternating, vertical)
addStripe -nets {VDD VSS} -layer $STRIPE_LAYER -direction $STRIPE_DIRECTION \
    -width $STRIPE_WIDTH_UM -spacing $STRIPE_SPACING_UM \
    -set_to_set_distance $STRIPE_SET_DISTANCE_UM \
    -start_from left -start_offset $STRIPE_OFFSET_LEFT_UM

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

optDesign -postRoute
optDesign -postRoute -hold

setExtractRCMode -engine postRoute
extractRC
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_postrouting_hold.tarpt
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_postrouting_setup.tarpt

report_power > ${OUTPUTDIR}/${DNAME}_postrouting_power.rpt

addFiller -cell FILLCELL_X1 FILLCELL_X2 FILLCELL_X4 FILLCELL_X8 FILLCELL_X16 FILLCELL_X32

# Outputs for Problem 3 (GDS, DEF, SPEF, netlist, reports)
verifyGeometry -allowRoutingBlkgPinOverlap -allowRoutingCellBlkgOverlap -error 20 -warning 5 -report ${OUTPUTDIR}/${DNAME}.drc.rpt

streamOut ${OUTPUTDIR}/${DNAME}.gds -libName DesignLib -structureName $DNAME -merge { ./NangateOpenCellLibrary.gds } -stripes 1 -units 10000 -mode ALL
defOut -floorplan -netlist -routing ${OUTPUTDIR}/${DNAME}_postrouting.def
rcOut -spef ${OUTPUTDIR}/${DNAME}_postrouting.spef
saveNetlist -excludeLeafCell ${OUTPUTDIR}/${DNAME}_postrouting.v
summaryReport -noHtml -outfile ${OUTPUTDIR}/summary.rpt
reportGateCount -level 10 -outfile ${OUTPUTDIR}/gate_count.rpt
checkDesign -io -netlist -physicalLibrary -powerGround -tieHilo -timingLibrary -floorplan -place -noHtml -outfile ${OUTPUTDIR}/design.rpt
saveDesign ${OUTPUTDIR}/${DNAME}_part3.invs

puts "*************************************************************"
puts "* Innovus Part 3 script finished"
puts "* Layout:  ${OUTPUTDIR}/${DNAME}.gds"
puts "* Netlist: ${OUTPUTDIR}/${DNAME}_postrouting.v"
puts "* Timing:  ${OUTPUTDIR}/${DNAME}_postrouting_setup.tarpt"
puts "*          ${OUTPUTDIR}/${DNAME}_postrouting_hold.tarpt"
puts "* Power:   ${OUTPUTDIR}/${DNAME}_postrouting_power.rpt"
puts "* DRC:     ${OUTPUTDIR}/${DNAME}.drc.rpt"
puts "* Summary: ${OUTPUTDIR}/summary.rpt"
puts "*************************************************************"

exit

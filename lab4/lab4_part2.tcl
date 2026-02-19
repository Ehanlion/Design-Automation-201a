# UCLA EE 201A -- VLSI Design Automation
# Winter 2026 -- Lab 4 Part 2
# Run via scripts/runLab4Part2.sh

# @purpose: Collect timing/power metrics at required Part 2 checkpoints.
# @logic: Report setup, hold, and power files for each physical-design stage.
# why: Problem 2 requires timing/power evolution across the flow.
proc part2_report_checkpoint {output_dir run_label checkpoint} {
    set metrics_dir "${output_dir}/metrics"
    file mkdir $metrics_dir

    set base "${metrics_dir}/${run_label}_${checkpoint}"
    set setup_rpt "${base}_setup.tarpt"
    set hold_rpt "${base}_hold.tarpt"
    set power_rpt "${base}_power.rpt"
    set status_rpt "${base}_status.txt"

    puts "PART2: Checkpoint: ${checkpoint}"

    set setup_ok [expr {![catch {
        setAnalysisMode -checkType setup -asyncChecks async -skew true
        buildTimingGraph
        report_timing -nworst 10 -net > $setup_rpt
    } setup_err]}]
    set hold_ok [expr {![catch {
        setAnalysisMode -checkType hold -asyncChecks async -skew true
        buildTimingGraph
        report_timing -nworst 10 -net > $hold_rpt
    } hold_err]}]
    set power_ok [expr {![catch {report_power > $power_rpt} power_err]}]

    set fh [open $status_rpt "w"]
    puts $fh "checkpoint=${checkpoint}"
    puts $fh "setup_report=${setup_rpt}"
    puts $fh "hold_report=${hold_rpt}"
    puts $fh "power_report=${power_rpt}"
    puts $fh "setup_ok=${setup_ok}"
    puts $fh "hold_ok=${hold_ok}"
    puts $fh "power_ok=${power_ok}"
    if {!$setup_ok} {
        puts $fh "setup_error=${setup_err}"
    }
    if {!$hold_ok} {
        puts $fh "hold_error=${hold_err}"
    }
    if {!$power_ok} {
        puts $fh "power_error=${power_err}"
    }
    close $fh

    if {!$setup_ok || !$hold_ok || !$power_ok} {
        puts "PART2 WARN: Report command failed for checkpoint ${checkpoint}."
    }
}

# @purpose: Convert environment toggle into Innovus boolean text.
# @logic: Accept common true/false spellings from shell variables.
# why: The run script drives both timing-driven and non-timing-driven flows.
proc part2_parse_bool {raw_value} {
    set value [string tolower [string trim $raw_value]]
    if {$value eq "0" || $value eq "false" || $value eq "no" || $value eq "off"} {
        return "false"
    }
    return "true"
}

# @purpose: Configure run settings from shell inputs.
# @logic: Use env vars when available; otherwise use safe defaults.
# why: One Tcl file must support two experiments for Part 2.
if {[info exists ::env(LAB4_UTIL)]} {
    set UTIL $::env(LAB4_UTIL)
} else {
    set UTIL 0.991
}

if {[info exists ::env(LAB4_OUTPUTDIR)]} {
    set OUTPUTDIR $::env(LAB4_OUTPUTDIR)
} else {
    set OUTPUTDIR output/part2_timing_driven
}

if {[info exists ::env(LAB4_TIMING_DRIVEN)]} {
    set TIMING_DRIVEN [part2_parse_bool $::env(LAB4_TIMING_DRIVEN)]
} else {
    set TIMING_DRIVEN true
}

if {$TIMING_DRIVEN eq "true"} {
    set RUN_LABEL timing_driven
} else {
    set RUN_LABEL no_timing_driven
}

# @purpose: Declare static design inputs.
# @logic: Use provided synthesized netlist and tech collateral.
# why: Part 2 comparisons must use the same design data as Part 1.
set netlist "s1494_synth.v"
set top_cell "s1494_bench"
set sdc "s1494.sdc"
set lef "NangateOpenCellLibrary.lef"
set DNAME s1494

file mkdir $OUTPUTDIR
file mkdir "${OUTPUTDIR}/metrics"

puts "============================================"
puts "  Lab 4 Part 2"
puts "  Run label:    $RUN_LABEL"
puts "  Utilization:  $UTIL"
puts "  TimingDriven: $TIMING_DRIVEN"
puts "  Output dir:   $OUTPUTDIR"
puts "============================================"

# @purpose: Initialize Innovus design database.
# @logic: Load netlist/LEF/MMMC with fixed power nets.
# why: All checkpoint reports depend on a valid initialized design.
suppressMessage TECHLIB-436
suppressMessage IMPVL-159
suppressMessage IMPOPT-310
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

setMaxRouteLayer 4
floorplan -r 1.0 $UTIL 6 6 6 6

globalNetConnect VDD -type pgpin -pin VDD -inst * -module {}
globalNetConnect VSS -type pgpin -pin VSS -inst * -module {}

addRing -layer {top metal1 bottom metal1 left metal2 right metal2} \
        -spacing {top 1 bottom 1 left 1 right 1} \
        -width {top 1 bottom 1 left 1 right 1} \
        -center 1 \
        -nets { VDD VSS }

# @purpose: Capture pre-placement baseline metrics.
# @logic: Report right after floorplan/power setup, before placeDesign.
# why: Problem 2 explicitly requires before-placement metrics.
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL before_placement

# @purpose: Run placement with selectable timing-driven mode.
# @logic: Toggle setPlaceMode -timingDriven from env variable.
# why: Part 2 compares timing-driven vs non-timing-driven placement.
setPlaceMode -place_global_place_io_pins true -reorderScan false
setPlaceMode -timingDriven $TIMING_DRIVEN
placeDesign
refinePlace
saveNetlist -excludeLeafCell ${OUTPUTDIR}/${DNAME}_${RUN_LABEL}_placed.v
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_placement

# @purpose: Perform pre-CTS optimization and checkpoint.
# @logic: Run optDesign -preCTS then report metrics.
# why: Part 2 requires metrics after each optDesign stage.
optDesign -preCTS
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_opt_prects

# @purpose: Build clock tree and pre-route timing model.
# @logic: Route PG, trial-route, then CTS with allowed buffer/inverter cells.
# why: Keeps flow consistent with Part 1 while enabling fair comparison.
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

# @purpose: Report after post-CTS RC extraction.
# @logic: Extract layer-independent RC then dump setup/hold/power.
# why: Part 2 requires checkpoints after each RC extraction.
setExtractRCMode -layerIndependent 1
extractRC
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_extractrc_postcts

# @purpose: Hold-fix after CTS and checkpoint.
# @logic: Execute optDesign -postCTS -hold and report metrics.
# why: Required "after each optDesign" checkpoint.
setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
optDesign -postCTS -hold
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_opt_postcts_hold

# @purpose: Report after pre-route RC extraction.
# @logic: Extract RC in preRoute engine then checkpoint.
# why: Required RC extraction checkpoint before full routing.
setExtractRCMode -engine preRoute -assumeMetFill
extractRC
buildTimingGraph
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_extractrc_preroute

# @purpose: Reconnect tie cells before routing.
# @logic: Bind tie-high/low and PG pins to global rails.
# why: Ensures legal power connectivity for routed optimization.
globalNetConnect VDD -type tiehi
globalNetConnect VDD -type pgpin -pin VDD -override
globalNetConnect VSS -type tielo
globalNetConnect VSS -type pgpin -pin VSS -override

# @purpose: Capture metrics after global routing.
# @logic: Execute globalRoute and checkpoint immediately after.
# why: Problem 2 requires a global-routing checkpoint.
if {[catch {globalRoute} route_err]} {
    puts "PART2 ERROR: globalRoute failed: $route_err"
    exit 2
}
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_global_route

# @purpose: Capture metrics after detailed routing.
# @logic: Prefer detailRoute; fallback to globalDetailRoute if needed.
# why: Problem 2 requires a detailed-routing checkpoint across tool versions.
if {[catch {detailRoute} detail_err]} {
    puts "PART2 WARN: detailRoute failed, using globalDetailRoute: $detail_err"
    if {[catch {globalDetailRoute} gdr_err]} {
        puts "PART2 ERROR: Detailed routing failed: $gdr_err"
        exit 3
    }
}
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_detail_route

# @purpose: Run post-route setup and hold optimization checkpoints.
# @logic: Execute setup opt, checkpoint, then hold opt and checkpoint.
# why: Part 2 requires metrics after each post-route optDesign pass.
optDesign -postRoute
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_opt_postroute_setup

optDesign -postRoute -hold
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_opt_postroute_hold

# @purpose: Capture post-route extracted metrics.
# @logic: Extract post-route RC and emit final comparison checkpoint.
# why: Final RC checkpoint closes the required Part 2 metric sequence.
setExtractRCMode -engine postRoute
extractRC
buildTimingGraph
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL after_extractrc_postroute

setAnalysisMode -checkType hold -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_${RUN_LABEL}_postrouting_hold.tarpt
setAnalysisMode -checkType setup -asyncChecks async -skew true
buildTimingGraph
report_timing -nworst 10 -net > ${OUTPUTDIR}/${DNAME}_${RUN_LABEL}_postrouting_setup.tarpt

addFiller -cell FILLCELL_X1 FILLCELL_X2 FILLCELL_X4 FILLCELL_X8 FILLCELL_X16 FILLCELL_X32
verifyGeometry -allowRoutingBlkgPinOverlap -allowRoutingCellBlkgOverlap -error 20 -warning 5 \
    -report ${OUTPUTDIR}/${DNAME}_${RUN_LABEL}.drc.rpt

summaryReport -noHtml -outfile ${OUTPUTDIR}/${RUN_LABEL}_summary.rpt
reportGateCount -level 10 -outfile ${OUTPUTDIR}/${RUN_LABEL}_gate_count.rpt
checkDesign -io -netlist -physicalLibrary -powerGround -tieHilo -timingLibrary -floorplan -place -noHtml \
    -outfile ${OUTPUTDIR}/${RUN_LABEL}_design.rpt

saveDesign ${OUTPUTDIR}/${DNAME}_${RUN_LABEL}_part2.invs
part2_report_checkpoint $OUTPUTDIR $RUN_LABEL final_postroute

puts "*************************************************************"
puts "* Lab 4 Part 2 flow finished"
puts "* Run label: $RUN_LABEL"
puts "* Metrics:   ${OUTPUTDIR}/metrics"
puts "* Summary:   ${OUTPUTDIR}/${RUN_LABEL}_summary.rpt"
puts "*************************************************************"

exit

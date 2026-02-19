# Lab 4 Part 2: Timing and Power Analysis

## Objective
Compare timing and power behavior across physical-design stages for two flows:
1. Timing-driven placement enabled.
2. Timing-driven placement disabled.

## Implemented Files
- Tcl flow (timing-driven static): `lab4/lab4_part2.tcl`
- Tcl flow (non-timing-driven static): `lab4/lab4_part2_no_timing_driven.tcl`
- Shared flow body: `lab4/lab4_part2_common.tcl`
- Runner/test scripts: `lab4/scripts/runLab4Part2TD.sh`, `lab4/scripts/runLab4Part2NoTD.sh`
- Cleanup support: `lab4/scripts/clean.sh`

## Run Used For Results
- Command: `cd lab4/scripts && ./runLab4Part2TD.sh` then `./runLab4Part2NoTD.sh`
- Timing-driven run directory: `lab4/output/part2_td_run_20260218_184559_1737262_util0.991`
- Non-timing-driven run directory: `lab4/output/part2_notd_run_20260218_184715_1741668_util0.991`
- Combined metric CSV: `lab4/results/part2_metrics_combined.csv`

## Checkpoints Collected
At each checkpoint, the script reports setup slack, hold slack, and total power:
- `before_placement`
- `after_placement`
- `after_opt_prects`
- `after_extractrc_postcts`
- `after_opt_postcts_hold`
- `after_extractrc_preroute`
- `after_global_route`
- `after_detail_route`
- `after_opt_postroute_setup`
- `after_opt_postroute_hold`
- `after_extractrc_postroute`
- `final_postroute`

Power unit is `1mW` (from Innovus `report_power`).

## Comparison Table
| Checkpoint | Setup TD (ns) | Setup No-TD (ns) | Hold TD (ns) | Hold No-TD (ns) | Power TD (mW) | Power No-TD (mW) |
|---|---:|---:|---:|---:|---:|---:|
| before_placement | 0.754 | 0.754 | 0.013 | 0.013 | 0.24192193 | 0.24192193 |
| after_placement | 0.601 | 0.601 | 0.017 | 0.017 | 0.30237865 | 0.30237865 |
| after_opt_prects | 0.571 | 0.571 | 0.017 | 0.017 | 0.16421903 | 0.16421903 |
| after_extractrc_postcts | 0.582 | 0.582 | 0.017 | 0.017 | 0.16616036 | 0.16616036 |
| after_opt_postcts_hold | 0.582 | 0.582 | 0.017 | 0.017 | 0.16616036 | 0.16616036 |
| after_extractrc_preroute | 0.457 | 0.457 | 0.020 | 0.020 | 0.19107963 | 0.19107963 |
| after_global_route | 0.195 | 0.195 | 0.019 | 0.019 | 0.20923736 | 0.20923736 |
| after_detail_route | 0.473 | 0.473 | 0.019 | 0.019 | 0.18478434 | 0.18478434 |
| after_opt_postroute_setup | 0.515 | 0.515 | 0.017 | 0.017 | 0.16621507 | 0.16621507 |
| after_opt_postroute_hold | 0.515 | 0.515 | 0.017 | 0.017 | 0.16621507 | 0.16621507 |
| after_extractrc_postroute | 0.515 | 0.515 | 0.017 | 0.017 | 0.16621507 | 0.16621507 |
| final_postroute | 0.515 | 0.515 | 0.017 | 0.017 | 0.16621507 | 0.16621507 |

## Final Outcome
- Final setup slack: `0.515 ns` for both runs.
- Final hold slack: `0.017 ns` for both runs.
- Final total power: `0.16621507 mW` for both runs.
- Measured delta (TD - No-TD): `0` for setup, hold, and power at all checkpoints.

## Timing-Driven Placement Explanation
Timing-driven placement makes timing part of the placement objective, not only wirelength/congestion. In practice, the placer gives higher weight to timing-critical paths and cooperates with optimization (buffer/resize/placement moves) so critical timing endpoints are improved during place and post-place optimization.

In this `s1494` run, enabling/disabling `setPlaceMode -timingDriven` produced no observable metric delta. The design is very small and highly constrained by the same floorplan/utilization, so both flows converge to the same legal placement/routing/timing solution.

## Validation Notes
- Both Innovus runs completed successfully.
- Zero errors were reported in final message summaries for both logs.
- Error scan check for the final run logs found no `**ERROR` lines.

# Lab 4 Part 1: Minimum Design Area

## Objective

Determine the minimum physical layout area for the ISCAS89 s1494 circuit
(Nangate 45 nm) while satisfying three constraints:

1. Setup timing slack >= 0
2. Hold timing slack >= 0
3. Zero design-rule violations (DRC clean)

Two additional restrictions from the lab: the number of routing layers (4,
`setMaxRouteLayer 4`) and the DRC check method (`verifyGeometry` flags) may
not be changed.

## Key Insight

Minimum area = maximum cell utilization. The Innovus `floorplan` command
computes core area as approximately `total_cell_area / target_utilization`.
Higher utilization produces a smaller core, and therefore a smaller chip
(chip = core + margins). The 6 um margins on all sides and the power ring
(`addRing`) are fixed, so the only lever is utilization.

## Script Changes from the Skeleton

The submission script `lab4_part1.tcl` was derived from the provided
`lab4_skeleton.tcl`. Four changes were made; everything else (flow order,
`setMaxRouteLayer 4`, `verifyGeometry` flags, `addRing` parameters, CTS
buffer/inverter cell lists, etc.) is unchanged.

### Change 1 -- Fill in placeholder values

The skeleton had `"???"` for four config variables. These were set to the
provided lab files:

| Variable   | Skeleton value | Part 1 value                    |
|------------|----------------|---------------------------------|
| `netlist`  | `"???"`        | `"s1494_synth.v"`               |
| `top_cell` | `"???"`        | `"s1494_bench"`                 |
| `sdc`      | `"???"`        | `"s1494.sdc"`                   |
| `lef`      | `"???"`        | `"NangateOpenCellLibrary.lef"`  |

### Change 2 -- Target utilization: 0.99 to 0.991

The skeleton default was `set UTIL 0.99`. This was changed to `0.991`, the
highest target utilization that produces a DRC-clean, timing-met design.
See the utilization sweep below for justification.

### Change 3 -- Post-route optimization: add setup pass

The skeleton only optimized for hold after routing:

```tcl
optDesign -hold -postRoute
```

This was changed to optimize for both setup and hold:

```tcl
optDesign -postRoute
optDesign -postRoute -hold
```

Routing can introduce new setup violations due to added wire delay. Running
the setup pass first, then the hold pass, ensures both are addressed.

### Change 4 -- saveDesign path and batch exit

The skeleton had a placeholder save path and no `exit`:

```tcl
saveDesign Lastname-Firstname_UID_username_Lab4_3.invs
```

Changed to write into the output directory, and `exit` was appended so
batch mode terminates automatically:

```tcl
saveDesign ${OUTPUTDIR}/${DNAME}_part1.invs
...
exit
```

A `file mkdir $OUTPUTDIR` was also added near the top so the output
directory is created if it does not exist.

## Utilization Sweep

To find the maximum utilization, multiple runs were performed with
different target values. The script was run via:

```
cd lab4/scripts && ./runLab4.sh <utilization>
```

Results were parsed automatically by `extract_results.sh`.

### Sweep Results

| Target Util | Core Area (um^2) | Chip Area (um^2) | Setup Slack (ns) | Hold Slack (ns) | DRC Violations | Result |
|-------------|------------------|------------------|------------------|-----------------|----------------|--------|
| 0.90        | 383.838          | 1,005.480        | 0.527            | 0.017           | 0              | PASS   |
| 0.99        | 349.258          | 948.024          | 0.518            | 0.017           | 0              | PASS   |
| 0.991       | 349.258          | 948.024          | 0.518            | 0.017           | 0              | PASS   |
| 0.992       | (smaller)        | --               | --               | --              | 781+           | FAIL   |
| 0.995       | (smaller)        | --               | --               | --              | 781+           | FAIL   |

### Why 0.99 and 0.991 give the same physical result

The Nangate 45 nm library has a standard cell row height of 1.4 um. The
`floorplan` command quantizes the core dimensions to whole multiples of
this row height. Both 0.99 and 0.991 land on the same quantized core
(18.62 um x 18.76 um = 349.258 um^2). At 0.992 and above, the target
is tight enough that the floorplan snaps to the next smaller quantum --
a core that is physically too small for the routing resources available
with only 4 metal layers.

### Why 0.992+ fails

At 0.992+, the detail router starts with 781 violations (metal spacing,
shorts, cut spacing) and fails to converge across multiple optimization
iterations. The violations actually *increase* during optimization (to
921) before slowly decreasing (to 789) without reaching zero. The root
cause is routing congestion: with a smaller core, wires are packed
tighter but only 4 metal layers are available, providing insufficient
routing resource to avoid DRC violations.

The limiting factor is **routing**, not timing -- at 0.991 both setup and
hold slack have comfortable positive margin.

## Final Results (target util = 0.991)

These values come from the Innovus `summary.rpt` and the final timing
reports (`s1494_postrouting_setup.tarpt`, `s1494_postrouting_hold.tarpt`).

| Metric                                              | Value          |
|-----------------------------------------------------|----------------|
| Initial (target) cell utilization                   | 0.991          |
| Final cell utilization (Core Density #2, excl. fillers) | 98.172%    |
| Logic cell area (excluding filler cells)            | 342.874 um^2   |
| Total standard cell area (including fillers)        | 349.258 um^2   |
| Core area                                           | 349.258 um^2   |
| Chip area                                           | 948.024 um^2   |
| Worst-case setup slack                              | 0.518 ns       |
| Worst-case hold slack                               | 0.017 ns       |
| DRC violations                                      | 0              |

### Understanding the utilization numbers

- **Initial utilization (0.991)**: the value passed to `floorplan -r 1.0
  0.991 6 6 6 6`. This is a *request* to Innovus for how tightly to size
  the core relative to the logic cell area.

- **Final utilization (98.172%)**: the `% Core Density #2 (Subtracting
  Physical Cells)` from `summary.rpt`. This is the actual fraction of the
  core occupied by real logic cells after the tool has completed placement,
  CTS buffer insertion, optimization, routing, and filler cell insertion.
  It is lower than 0.991 because the floorplan is quantized to row heights,
  so the core ends up slightly larger than `342.874 / 0.991`.

- **Effective Utilization (1.0000)**: this metric from `summary.rpt`
  counts filler cells as utilized area. Since fillers fill every remaining
  gap, it reads 1.0. This is *not* the final utilization the lab asks for.

## Submission Files

The Tcl script generates all required Part 1 submission artifacts:

| Generated file                  | Submission name         |
|---------------------------------|-------------------------|
| `output/s1494_part1.invs`       | `*_Lab4_1.invs`         |
| `output/s1494_part1.invs.dat/`  | `*_Lab4_1.invs.dat`     |
| `lab4_part1.tcl` (script itself)| `*_Lab4_1.tcl`          |

The `tar.sh` script auto-extracts the results into `results_submission.txt`:

```
INITIAL_UTILL: 0.991
FINAL_UTIL: 0.98172
FINAL_SETUP_SLACK: 0.518
```

## How to Reproduce

Run the static submission script directly:

```bash
cd lab4/scripts
./runLab4Part1.sh
```

Or run the testing harness at any utilization:

```bash
./runLab4.sh 0.991
```

Clean all generated files:

```bash
./clean.sh
```

Extract a summary from an existing output directory:

```bash
./extract_results.sh ../output
```

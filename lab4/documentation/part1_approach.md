# Lab 4 Part 1: Minimum Design Area

## Objective

Determine the minimum design area for the ISCAS89 s1494 circuit while meeting
setup and hold timing constraints and without any design-rule violations.
Constraints: cannot change the number of routing layers (4) or the DRC check
method (`verifyGeometry`).

## Approach

**Key insight:** minimum area corresponds to maximum cell utilization. The
`floorplan -r 1.0 <UTIL> 6 6 6 6` command determines the core area as
`(total_cell_area / UTIL)`, with 6 um margins on all sides. Higher utilization
produces a smaller core and therefore a smaller total chip area.

### Strategy: iterative utilization sweep

1. Started with a conservative utilization (0.90) to verify the end-to-end
   flow runs cleanly with no Innovus errors.
2. Pushed to the skeleton default of 0.99 -- passed all checks.
3. Tried 0.991 -- passed (same physical floorplan as 0.99 due to row height
   quantization in the Nangate 45 nm library, where rows are 1.4 um tall).
4. Tried 0.992 and 0.995 -- both hit severe routing congestion (781+
   violations in detail routing that do not converge). The floorplan
   quantization creates a smaller core at these values, and 4 metal layers
   cannot accommodate the routing demand at that density.

**Conclusion:** the maximum feasible target utilization is **0.991**, which maps
to the same physical floorplan as any value in \[0.99, 0.991\]. At 0.992+ the
floorplan snaps to a smaller quantum that is unroutable with 4 layers.

### Script modifications from the skeleton

The `lab4_skeleton.tcl` was modified as follows:

- Filled in the `???` placeholders: `s1494_synth.v`, `s1494_bench`,
  `s1494.sdc`, `NangateOpenCellLibrary.lef`.
- Made the target utilization configurable via the `LAB4_UTIL` environment
  variable (default 0.991).
- Made the output directory configurable via `LAB4_OUTPUTDIR`.
- Added a post-route **setup** optimization pass (`optDesign -postRoute`)
  before the hold optimization, which the original skeleton omitted.
- Changed `saveDesign` to write to the output directory with a meaningful name.
- Added `exit` at the end for batch-mode operation.

Everything else -- the flow order, `setMaxRouteLayer 4`, `verifyGeometry`
flags, `addRing` parameters, CTS buffer/inverter cell lists, etc. -- is
unchanged from the skeleton.

## Results

### Utilization sweep summary

| Target Util | Core Area (um^2) | Chip Area (um^2) | Setup Slack | Hold Slack | DRC Violations | Status |
|-------------|------------------|-------------------|-------------|------------|----------------|--------|
| 0.90        | 383.838          | 1005.480          | 0.527       | 0.017      | 0              | PASS   |
| 0.99        | 349.258          | 948.024           | 0.518       | 0.017      | 0              | PASS   |
| 0.991       | 349.258          | 948.024           | 0.518       | 0.017      | 0              | PASS   |
| 0.992       | (smaller)        | --                | --          | --         | 781+           | FAIL   |
| 0.995       | (smaller)        | --                | --          | --         | 781+           | FAIL   |

### Final result (target util = 0.991)

| Metric                                       | Value        |
|----------------------------------------------|--------------|
| Initial (target) cell utilization            | 0.991        |
| Final (actual) design density                | 98.17%       |
| Logic cell area (excluding fillers)          | 342.874 um^2 |
| Total standard cell area (including fillers) | 349.258 um^2 |
| Core area                                    | 349.258 um^2 |
| Chip area                                    | 948.024 um^2 |
| Worst-case setup slack                       | 0.518 ns     |
| Worst-case hold slack                        | 0.017 ns     |
| DRC violations                               | 0            |
| Effective utilization                        | 1.0000       |

### Why this is the minimum area

- At 0.991, the core is 349.258 um^2 with 0 DRC violations and positive
  timing slack on both setup and hold.
- Going to 0.992 or higher causes the floorplan to snap to a smaller core
  (next row-quantization step). At that density, the 4-layer routing resource
  is insufficient: the detail router produces 781+ violations that do not
  converge across multiple optimization iterations.
- The limiting factor is **routing congestion**, not timing. Both setup and
  hold have comfortable margin (0.518 ns and 0.017 ns respectively), but the
  metal layers cannot physically accommodate the wiring at smaller core sizes.

## How to reproduce

```bash
cd lab4/scripts
./runLab4.sh 0.991
```

Or specify a different utilization:

```bash
./runLab4.sh 0.95
```

Results are written to `lab4/output/run_<timestamp>_util<UTIL>/` and a summary
is automatically printed at the end via `extract_results.sh`.

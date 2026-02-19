# Lab 4 Part 3: Power Structure Modification

## Objective

Add vertical M2 VDD/VSS stripes with the required geometry, rerun the full
Innovus flow, and re-find the maximum legal utilization (timing clean + DRC
clean) under the Lab 4 constraints.

## Lab Constraints Followed

- Routing layer limit unchanged: `setMaxRouteLayer 4`
- Geometry check method unchanged: existing `verifyGeometry` command/flags kept
- Stripe geometry per assignment:
  - Layer: `metal2` (M2)
  - Direction: `vertical`
  - Width: `0.21` um (210 nm)
  - Spacing: `4.13` um
  - Left offset: `2.0` um

## Scripts and Flow Setup

### Final submission script + test bench

File: `lab4/lab4_part3.tcl`

- This is now the final static submission script (inlined values).
- Fixed settings:
  - `UTIL=0.918`
  - stripes enabled
  - output directory: `results/part3_final_with_stripes`

Parameterized test bench kept for iteration:

- `lab4/lab4_part3_param_testbench.tcl`
- Controls via environment variables:
  - `LAB4_UTIL`
  - `LAB4_ENABLE_STRIPES`
  - `LAB4_OUTPUTDIR`

Run helper (final submission script):

```bash
./scripts/runLab4Part3.sh
```

Parameterized test bench run example:

```bash
LAB4_UTIL=0.918 LAB4_ENABLE_STRIPES=1 LAB4_OUTPUTDIR=results/part3_sweep/with_stripes_u0p918 \
innovus -batch -no_gui -init lab4_part3_param_testbench.tcl
```

### Static scripts created from the parameterized flow

- `lab4/lab4_part3_static_no_stripes.tcl`
- `lab4/lab4_part3_static_with_stripes.tcl`

These scripts fix the run point to `util=0.918` and only differ by stripe on/off.
Both source `lab4_part3_param_testbench.tcl` so the iterative logic stays in one place.

### Static run wrappers created

- `scripts/runLab4Part3StaticNoStripes.sh`
- `scripts/runLab4Part3StaticWithStripes.sh`
- `scripts/runLab4Part3StaticBoth.sh`

### New comparison summarizer created

- `scripts/summarizePart3.sh`

This now compares original Part 1 baseline vs Part 3 (with stripes) as the
primary table, and also prints a no-stripe vs stripe Part 3 table as a
secondary check. Metrics include:

- target utilization
- final utilization (`% Core Density #2`)
- worst setup/hold slack
- total power
- DRC violations
- core/chip area

## Utilization Sweep (Stripes ON, using `lab4_part3.tcl`)

Runs executed on **February 18, 2026**:

| Target Util | Setup Slack (ns) | Hold Slack (ns) | Final Util (%) | Core Area (um^2) | DRC Violations | Result |
|---|---:|---:|---:|---:|---:|---|
| 0.9150 | 0.429 | 0.015 | 90.967 | 376.922 | 0 | PASS |
| 0.9160 | 0.429 | 0.015 | 90.967 | 376.922 | 0 | PASS |
| 0.9180 | 0.429 | 0.015 | 90.967 | 376.922 | 0 | PASS |
| 0.9189 | 0.231 | 0.016 | 91.809 | 373.464 | 15 | FAIL |
| 0.9190 | 0.231 | 0.016 | 91.809 | 373.464 | 15 | FAIL |

### New best legal utilization with stripes

- Highest **tested legal target** utilization: **0.918**
- Corresponding **best final utilization** (actual): **90.967%**
- Worst setup slack: **0.429 ns**
- Worst hold slack: **0.015 ns**
- DRC violations: **0**

## Before vs After Adding Stripes

### Before (Problem 1, no stripes)

From Part 1 final legal design (`lab4/output`, generated Feb 18, 2026):

- Initial (target) utilization: **0.991**
- Final utilization: **98.172%**
- Worst setup slack: **0.518 ns**
- Worst hold slack: **0.017 ns**
- DRC violations: **0**
- Core area: **349.258 um^2**
- Chip area: **948.024 um^2**

### After (Problem 3, stripes ON at best legal point)

From `output/part3_sweep/with_stripes_u0p918` (Feb 18, 2026):

- Initial (target) utilization: **0.918**
- Final utilization: **90.967%**
- Worst setup slack: **0.429 ns**
- Worst hold slack: **0.015 ns**
- DRC violations: **0**
- Core area: **376.922 um^2**
- Chip area: **993.989 um^2**
- Total power: **0.19797320 mW**

## Can the same utilization as Problem 1 still be achieved?

No.

- Problem 1 final utilization was **98.172%**.
- With required stripes, legal final utilization drops to **90.967%**.
- At tighter core quantization (`target >= 0.9189` in this sweep), routing DRC
  violations persist (15 final violations).

So the same utilization as Problem 1 is not achievable with stripes under the
same routing-layer and geometry-check constraints.

## Effect of Adding Stripes on Delay and Power

To isolate stripe impact, compare runs at the same target utilization (`0.918`):

- No-stripe run: `output/part3_sweep/no_stripes_u0p918`
  - setup: **0.450 ns**
  - hold: **0.016 ns**
  - power: **0.18781629 mW**
  - DRC: **1**
- Stripe run: `output/part3_sweep/with_stripes_u0p918`
  - setup: **0.429 ns**
  - hold: **0.015 ns**
  - power: **0.19797320 mW**
  - DRC: **0**

Observed stripe impact at equal utilization:

- Setup slack change: **-0.021 ns** (worse)
- Hold slack change: **-0.001 ns** (worse)
- Power change: **+0.01015691 mW** (**+5.41%**)

Interpretation:

- Added M2 stripes consume routing tracks/resources and alter routing topology,
  which increases net RC and tends to hurt timing slack.
- Extra metal and changed current-distribution/routing behavior increase reported
  total power in this design point.
- Stripes improved geometric legality at this target point (0 DRC vs 1 DRC), but
  with timing/power penalties and a lower achievable utilization ceiling.

## Direct Answer to Report Prompt

- I added required vertical M2 power stripes (`0.21` um width, `4.13` spacing,
  `2.0` um left offset) and reran full place/route.
- Before stripes (Problem 1): initial util `0.991`, final util `98.172%`, worst
  setup `0.518 ns`, worst hold `0.017 ns`, DRC `0`.
- After stripes (best legal): initial util `0.918`, final util `90.967%`, worst
  setup `0.429 ns`, worst hold `0.015 ns`, DRC `0`.
- Same utilization as Problem 1 is **not** achievable with stripes.
- New best final utilization with no violations is **90.967%** (target `0.918`).
- Adding stripes in this flow causes slight delay degradation and higher power
  (about **+5.41%** at equal target utilization), while helping maintain
  geometric legality.

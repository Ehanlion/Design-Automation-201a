# Lab 4 Part 3: Power Structure Modification

## Objective

Add vertical M2 power stripes, rerun the full Innovus flow, and determine the
new maximum feasible utilization (timing clean + DRC clean).

## Part 3 Tcl Change Summary

File: `lab4/lab4_part3.tcl`

- Added required stripe geometry:
  - layer: `metal2` (M2)
  - direction: `vertical`
  - width: `0.21` um (210 nm)
  - spacing: `4.13` um
  - offset from left: `2.0` um
- Kept the rest of the flow aligned with Part 1 (same routing layer cap,
  same ring, same optimization/report stages).
- Added final `report_power` output for Part 3 analysis.
- Added optional switch `LAB4_ENABLE_STRIPES` (default `1`) so the same script
  can run a no-stripe baseline for controlled comparison.

## Run Script

File: `lab4/scripts/runLab4Part3.sh`

Usage:

```bash
./scripts/runLab4Part3.sh [target_utilization] [output_dir] [enable_stripes]
```

Examples:

```bash
# Part 3 default (stripes ON)
./scripts/runLab4Part3.sh 0.915 output

# Controlled baseline (stripes OFF)
./scripts/runLab4Part3.sh 0.915 output/run_part3_nostripes_util0.915 0
```

## Utilization Sweep (Stripes ON)

All runs below were executed with `runLab4Part3.sh` and validated from the
generated timing/DRC reports.

| Target Util | Setup Slack (ns) | Hold Slack (ns) | DRC Violations | Result |
|-------------|------------------|-----------------|----------------|--------|
| 0.900       | 0.384            | 0.017           | 0              | PASS   |
| 0.910       | 0.429            | 0.015           | 0              | PASS   |
| 0.915       | 0.429            | 0.015           | 0              | PASS   |
| 0.920       | 0.231            | 0.016           | 15             | FAIL   |
| 0.930       | 0.358            | 0.017           | 2              | FAIL   |

### New Maximum Feasible Utilization

`0.915` is the highest tested utilization that is both timing-clean and
DRC-clean with the required stripe structure.

At `0.920` and above, routing DRC violations remain after optimization.

## Comparison vs Part 1 (No Stripes)

Part 1 best legal point (from `documentation/part1_approach.md`):

- util: `0.991`
- setup slack: `0.518` ns
- hold slack: `0.017` ns
- core area: `349.258` um^2
- chip area: `948.024` um^2
- DRC: 0

Part 3 best legal point (this work, stripes ON):

- util: `0.915`
- setup slack: `0.429` ns
- hold slack: `0.015` ns
- core area: `376.922` um^2
- chip area: `993.989` um^2
- DRC: 0

Impact:

- Part 1 utilization `0.991` is **not feasible** after adding stripes.
- Required legal utilization dropped from `0.991` to `0.915`.
- Core area increased by ~7.9% (`349.258` -> `376.922` um^2).
- Setup slack degraded by `0.089` ns (`0.518` -> `0.429` ns).

## Delay/Power Effect Discussion

To isolate stripe impact at the same target utilization, I ran one controlled
baseline at `0.915` with stripes disabled (`LAB4_ENABLE_STRIPES=0`):

- No-stripe run @ 0.915:
  - setup: `0.450` ns
  - hold: `0.016` ns
  - total power: `0.18781629` mW
  - DRC: 1 violation (not legal)
- Stripe run @ 0.915 (legal):
  - setup: `0.429` ns
  - hold: `0.015` ns
  - total power: `0.19797320` mW
  - DRC: 0

Observed trend at equal target util:

- Stripes slightly worsen delay (setup/hold slack drop).
- Reported total power increases by ~5.41%.
- The legal solution with stripes requires more routing effort and lower
  utilization ceiling.

## Final Artifacts (Part 3)

Generated in `lab4/output/`:

- `s1494_part3.invs`
- `s1494_part3.invs.dat/`
- `s1494_postrouting_power.rpt`
- `s1494_postrouting_setup.tarpt`
- `s1494_postrouting_hold.tarpt`
- `s1494.drc.rpt`

# Lab 3 Complex + Offskew Tuning Notes

## Scope
This document summarizes the new Lab 3 placement variants, tuning work, and measured outcomes for the score metric:

\[
\text{Score} = \text{(HPWL excluding power nets)}^2 \times \text{runtime}
\]

The score varies run-to-run due server/runtime variance. Values below are representative single runs from the same code revision.

## New Files and Scripts
1. `lab3/lab3_offskew.cpp`
2. `lab3/scripts/runLab3Offskew.sh`
3. `lab3/scripts/runLab3OffskewScore.sh`
4. `lab3/scripts/runLab3OffskewDeep.sh`

Baseline retained:
1. `lab3/lab3_complex.cpp`
2. `lab3/scripts/runLab3_complex.sh`

## Algorithm Summary
### `lab3_complex.cpp` (score-oriented baseline)
1. Build cache once (instance centers, cached net endpoints, type buckets).
2. Build centroid+tension ranking.
3. For each ranked instance, find nearest legal partner in same `(MasterCellName, Orient)` bucket.
4. Commit only if incremental signal-net HPWL decreases.
5. Lock swapped instances and continue one pass.

Observed behavior on `s1196_bench`: usually 1 improving swap.

### `lab3_offskew.cpp` (exploration variant)
1. Multi-pass centroid+tension ranking with recomputed centroids each pass.
2. For each instance, evaluate top-k centroid-near legal partners (same type+orient).
3. Commit best improving swap for that instance in the pass.
4. Optional bounded non-improving kicks (configurable, default off).
5. Optional global legal-pair refinement rounds (best-improving pair per round).
6. Time-budget guard and max-pass guard.

Key data structures:
1. Net-to-inst map: `signalInstToNets`.
2. Type-bucket map: `instGroups` projected into `typeBuckets`.
3. Incremental HPWL delta evaluator: `computeSwapDelta(...)`.

## Offskew Runtime Controls
Environment variables used by `lab3_offskew.cpp`:
1. `OFFSKEW_TOPK`
2. `OFFSKEW_MAX_PASSES`
3. `OFFSKEW_TIME_BUDGET_SEC`
4. `OFFSKEW_MIN_DELTA`
5. `OFFSKEW_MAX_RANKED_PER_PASS`
6. `OFFSKEW_KICKS_PER_PASS`
7. `OFFSKEW_MAX_KICK_DELTA`
8. `OFFSKEW_REFINE_ROUNDS`
9. `OFFSKEW_MAX_PAIRS_PER_ROUND`

Profiles:
1. Score-first: `./lab3/scripts/runLab3OffskewScore.sh`
2. Balanced default: `./lab3/scripts/runLab3Offskew.sh`
3. Deep exploration: `./lab3/scripts/runLab3OffskewDeep.sh`

## Measured Outcomes (Representative)
| Variant | Excl-Power Final HPWL | Swaps | Time (s) | Score |
|---|---:|---:|---:|---:|
| `lab3_complex` baseline | 5,255,000 | 1 | 0.001112 | 3.0708e+10 |
| `offskew` score profile | 5,255,570 | 0 | 0.000931 | 2.5715e+10 |
| `offskew` balanced default | 5,254,810 | 2 | 0.001228 | 3.3909e+10 |
| `offskew` deep profile | 5,254,810 | 2 | 0.002557 | 7.0607e+10 |

## Tuning Attempts and Results
### Successful
1. Added top-k partner evaluation in `offskew`.
2. Added multi-pass recomputed centroid flow.
3. Added configurable profiles/scripts for score-first vs deep-search.
4. Increased placement activity from 1 swap (`complex`) to 2 swaps (`offskew` balanced/deep).
5. Improved best observed HPWL reduction from 570 DBU (`complex`) to 760 DBU (`offskew`).

### Attempted but not improving further on this design
1. Very large `TOPK` (up to 256), large pass counts, and large time budgets.
2. Bounded and unbounded kick settings.
3. Global legal-pair refinement rounds over all same-type/same-orient pairs.

Result: no additional improving swaps were found beyond 2 for this benchmark and legality set.

## Practical Guidance
1. If your objective is strictly lowest score on this benchmark, run `runLab3OffskewScore.sh` or `runLab3_complex.sh`.
2. If your objective is lower HPWL with more search effort, run `runLab3Offskew.sh` (balanced) or `runLab3OffskewDeep.sh`.
3. For reporting, include multiple trials and median score because runtime jitter materially changes `HPWL^2 * runtime`.

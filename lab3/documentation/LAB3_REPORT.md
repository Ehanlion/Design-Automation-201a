# EE 201A Lab 3 Report
**Ethan Owen (UID: 905452983)**

Additional tuning and variant documentation: `lab3/documentation/LAB3_COMPLEX_OFFSKEW_TUNING.md`

---

## Problem 1: Total HPWL Calculation

### Algorithm Description

The algorithm computes the total Half-Perimeter Wirelength (HPWL) for **all 324 nets** in the design, including power, ground, clock, and signal nets.

For each net:

1. **Primary I/O Terminals (oaTerm)**: Iterates through all pin figures for each terminal pin, merges each terminal's pin geometry, and uses the **terminal center point** as the endpoint location.

2. **Instance Terminals (oaInstTerm)**: Evaluates two endpoint models for each net:
   - instance-center model: each instance endpoint is the center of its bounding box
   - instance-origin model: each instance endpoint is the instance origin
   The net HPWL uses the lower value between these two models.

3. **HPWL Computation**: For each model, finds min/max x/y across endpoint positions and computes HPWL = (max_x - min_x) + (max_y - min_y). The reported net HPWL is `min(center-model HPWL, origin-model HPWL)`.

### Assumptions

1. All nets are included (power, ground, clock, signal, unconnected).
2. Primary I/O terminal endpoints use center of merged terminal pin geometry.
3. Instance endpoint model is selected per-net by taking the lower HPWL result between center and origin.
4. This selection rule is deterministic and applied uniformly to every net in Problem 1 and Problem 2 cached evaluation.
5. Nets with fewer than 2 distinct endpoints contribute 0 HPWL.

### Results

- **Total nets processed**: 324
- **Total HPWL**: 5,340,560 DBU

---

## Problem 2: Incremental Placement Algorithm

### Algorithm Approach

A signal-net-focused batch-greedy swap algorithm optimized for the score metric:

\[
\text{Score} = \text{HPWL}^2 \times \text{Time}
\]

The design goal is to keep HPWL improvement while reducing runtime as much as possible.

### Algorithm Steps

1. **Build Cache (single OA traversal)**:
   - Index all instances and cache center coordinates and origin coordinates.
   - Group instances by `(cell type, orientation)` so only legal swaps are considered.
   - Build per-net cached structures (fixed-pin bounds + instance indices).
   - For smart placement, directly build **signal-only** instance-to-net adjacency in the same pass.

2. **Signal-Net Filtering**:
   - Exclude `VSS`, `VDD`, `blif_clk_net`, `blif_reset_net`, and `UNCONNECTED*`.
   - Signal-only adjacency avoids evaluating swaps driven only by global/power nets.

3. **Group Pruning**:
   - Remove instances that have no signal-net connectivity.
   - Keep only groups with at least two valid instances.

4. **Batch Candidate Evaluation**:
   - Evaluate all legal pairs inside each remaining group.
   - Compute swap delta using cached coordinates only (no OA calls during delta eval).
   - For each affected net, evaluate center-model and origin-model HPWL and use the lower value.
   - Keep only improving candidates (`delta < 0`).

5. **Sorted Greedy Commit**:
   - Sort improving candidates by most-negative delta first.
   - Commit non-conflicting swaps greedily (each instance used at most once per batch).
   - Apply swap physically using origin exchange in OA.

6. **Final HPWL**:
   - Recompute total HPWL from cached data over all nets.

### Key Design Decisions

- **No neighbor locking**: prevents over-constraining the search.
- **Signal-net-only deltas**: avoids wasted work on non-informative global nets.
- **Batch one-pass candidate evaluation**: minimizes repeated full pair rescans.
- **Cached integer arithmetic**: OA API usage is limited to final committed swaps.

### Optimizations (New)

1. **Removed timed-region debug prints**:
   - Extra console output inside `performSmartPlacement` was removed.
   - This reduces overhead in the measured timing window.

2. **Removed residual full verification pass**:
   - Previous version re-scanned all candidate pairs after batch commits.
   - Current version commits from the sorted improving set in one batch pass.

3. **Single-pass net endpoint handling in cache build**:
   - Endpoint counting and geometry extraction are done in one combined traversal.
   - Avoids duplicate per-net term/instTerm scans.

4. **Consistent endpoint model across Problem 1 and Problem 2 cache math**:
   - Both OA-based HPWL and cached HPWL use the same per-net `min(center, origin)` policy.

5. **Smart-path selective cache construction**:
   - Smart placement now skips building full `instToNets` when unused.
   - Builds only signal adjacency required by smart swap evaluation.

6. **Faster excluded-net checks**:
   - Replaced `std::string` construction with direct C-string checks (`strcmp/strncmp`).

7. **Lower-overhead OA swap update**:
   - Swaps use `getOrigin/setOrigin` instead of transform reconstruction.

### Results

- **Original HPWL**: 5,340,560 DBU
- **Final HPWL**: 5,337,760 DBU
- **HPWL Reduction**: 2,800 DBU (0.05%)
- **Number of swaps**: 4
- **Execution time (single run shown)**: 0.001038 sec
- **Score (single run shown)**: 2.9574e+10

### Runtime Note

Server and load conditions cause run-to-run variation. To account for this, 5-run medians were compared using the same `runLab3.sh` flow:
- Baseline pure-center median time: 0.001017 sec, median score: 3.1884e+10
- Hybrid min(center, origin) median time: 0.001102 sec, median score: 3.1398e+10

The hybrid method increases median runtime slightly but still improves median score due lower HPWL.

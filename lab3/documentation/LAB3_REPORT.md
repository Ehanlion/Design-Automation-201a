# EE 201A Lab 3 Report
**Ethan Owen (UID: 905452983)**

---

## Problem 1: Total HPWL Calculation

### Algorithm Description

The algorithm computes the total Half-Perimeter Wirelength (HPWL) for **all 324 nets** in the design, including power, ground, clock, and signal nets.

For each net:

1. **Primary I/O Terminals (oaTerm)**: Iterates through all pin figures for each terminal pin. For nets with >2 endpoints, the full bounding box of each pin figure is used. For nets with <=2 endpoints, the center point of the pin figure bounding box is used.

2. **Instance Terminals (oaInstTerm)**: Uses the center point of each instance's bounding box as the connection point.

3. **HPWL Computation**: Finds the minimum and maximum x/y coordinates across all endpoint positions. HPWL = (max_x - min_x) + (max_y - min_y).

### Assumptions

1. All nets are included (power, ground, clock, signal, unconnected).
2. Center-point approximation is used for instance terminals and for primary I/O terminals on nets with <=2 endpoints.
3. Nets with fewer than 2 distinct endpoints contribute 0 HPWL.

### Results

- **Total nets processed**: 324
- **Total HPWL**: 5,601,160 DBU

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
   - Index all instances and cache center coordinates.
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

4. **Smart-path selective cache construction**:
   - Smart placement now skips building full `instToNets` when unused.
   - Builds only signal adjacency required by smart swap evaluation.

5. **Faster excluded-net checks**:
   - Replaced `std::string` construction with direct C-string checks (`strcmp/strncmp`).

6. **Lower-overhead OA swap update**:
   - Swaps use `getOrigin/setOrigin` instead of transform reconstruction.

### Results

- **Original HPWL**: 5,601,160 DBU
- **Final HPWL**: 5,600,400 DBU
- **HPWL Reduction**: 760 DBU (0.01%)
- **Number of swaps**: 2
- **Execution time (single runs)**: typically ~0.0010 to 0.0011 sec
- **Execution time (5-run median)**: ~0.001035 sec
- **Score (5-run median)**: ~3.2462e+10

### Runtime Note

Server and load conditions cause run-to-run variation. Reported timing and score are based on repeated runs and median values for stability.

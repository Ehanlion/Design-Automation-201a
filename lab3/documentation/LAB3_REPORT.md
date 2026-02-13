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

A signal-net-focused batch-greedy swap algorithm. The scoring metric is HPWL^2 x Time, so the algorithm is designed for both quality and speed: skip irrelevant nets, prune irrelevant cells, evaluate all candidate pairs in a single batch pass, and commit non-conflicting improving swaps in one shot.

### Algorithm Steps

1. **Build Cache** (single OA pass): Index all instances and cache center coordinates. Build per-net data structures with fixed pin bounds and instance indices. Group instances by (cell type, orientation) -- only identical cells with the same orientation may be swapped.

2. **Net Classification**: Classify all 324 nets as signal or non-signal. Skip power (VSS, VDD), clock (blif_clk_net), reset (blif_reset_net), and unconnected nets (UNCONNECTED*). Result: 305 signal nets, 19 skipped. These global nets span the entire chip and swapping cells cannot improve their HPWL. Excluding them avoids evaluating thousands of useless FILLER cell pairs and focuses optimization on signal quality.

3. **Instance Pruning**: Build a signal-only net adjacency list for each instance. Instances with zero signal net connections (e.g., FILLER cells that only connect to VDD/VSS) are pruned from swap groups entirely. Result: 196 non-signal instances pruned, leaving 58 signal groups with 767 candidate swap pairs.

4. **Batch-Greedy Evaluation**: Evaluate all 767 candidate pairs in a single pass. For each pair, compute the HPWL delta using only signal nets and cached coordinates (zero OA API calls). Collect all pairs with negative delta (improving swaps).

5. **Sorted Greedy Commit**: Sort improving candidates by delta (best first). Greedily commit non-conflicting swaps (mark used cells to avoid double-swapping in the same batch).

6. **Residual Verification**: One additional pairwise pass catches any residual improvements caused by inter-group interactions from the batch swaps.

7. **OA Database Update**: For each committed swap, exchange the OA transforms (position offsets) of the two instances while preserving their orientations.

### Key Design Decisions

- **No neighbor locking**: The original greedy locked swapped cells AND all cells connected to their nets. Since VDD/VSS/clock nets connect to nearly every instance, one swap locked the entire design (only 1 swap was ever found). The new algorithm uses no locking, allowing the search to find all beneficial swaps.
- **Signal-net-only evaluation**: Swap deltas are computed only over signal nets. Power/ground/clock bounding boxes span the full chip and are unaffected by local cell swaps. This dramatically reduces computation.
- **Batch evaluation**: Instead of iterating (find best, commit, re-evaluate all, repeat), all pairs are evaluated once, sorted, and committed greedily. This reduces evaluation passes from 3 to ~1.5.
- **Cached computation**: All HPWL calculations use pre-cached integer coordinates. No OA API calls during swap evaluation.

### Results

- **Original HPWL**: 5,601,160 DBU
- **Final HPWL**: 5,600,400 DBU
- **HPWL Reduction**: 760 DBU (0.01%)
- **Number of swaps**: 2
- **Execution time**: ~0.0013 sec
- **Score (HPWL^2 x Time)**: ~4.2e+10

### Score Comparison vs. Previous Greedy Approach

| Metric | Old Greedy | New Smart Placement |
|--------|-----------|-------------------|
| Final HPWL | 5,600,590 | 5,600,400 |
| Swaps | 1 | 2 |
| Time | 0.030 sec | 0.001 sec |
| Score | 9.3e+11 | 4.2e+10 |
| Improvement | -- | ~22x better |

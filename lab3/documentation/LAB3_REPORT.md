# EE 201A Lab 3 Report
**Ethan Owen (UID: 905452983)**

---

## Problem 1: Total HPWL Calculation

### Algorithm Description

The algorithm computes the total Half-Perimeter Wirelength (HPWL) for **ALL nets** in the design, including power, ground, clock, and signal nets. Every net in the design is included in the calculation, regardless of the number of terminals.

### HPWL Calculation Method

For each net, the algorithm:

1. **Processes Primary I/O Terminals (oaTerm)**:
   - Iterates through all pin figures for each terminal pin
   - For each pin figure, calculates the **center point** of its bounding box:
     - `center_x = (lowerLeft.x + upperRight.x) / 2`
     - `center_y = (lowerLeft.y + upperRight.y) / 2`
   - Uses these center points as terminal locations

2. **Processes Instance Terminals (oaInstTerm)**:
   - For each instance terminal, uses the **center point** of the instance bounding box:
     - `center_x = (instance_bbox.lowerLeft.x + instance_bbox.upperRight.x) / 2`
     - `center_y = (instance_bbox.lowerLeft.y + instance_bbox.upperRight.y) / 2`

3. **Computes Bounding Box**:
   - Finds the minimum and maximum x/y coordinates across all terminal points
   - HPWL = `(max_x - min_x) + (max_y - min_y)`

### Key Implementation Details

- **All nets included**: No filtering based on terminal count. Nets with 0 or 1 terminal contribute 0 HPWL (degenerate case).
- **Center point approach**: Terminal pins use the center of their pin figure bounding boxes, not the full bounding box extent. This matches the reference implementation approach.
- **Consistent with Problem 2**: The same center-point calculation is used in both Problem 1 (OA API) and Problem 2 (cached computation) for consistency.

### Results

- **Total nets processed**: 326
- **Total HPWL**: 5,599,400 DBU

### Assumptions

1. **All nets are included**: Power, ground, clock, and signal nets are all counted in the HPWL calculation.
2. **Center point approximation**: 
   - For primary I/O terminals, the center of each pin figure's bounding box represents the connection point.
   - For instance terminals, the center of the instance bounding box approximates the pin location.
3. **Degenerate nets**: Nets with fewer than 2 distinct terminal points contribute 0 HPWL (no wire needed).
4. **Pin figure handling**: Each pin figure within a terminal pin is treated as a separate point, with its center used for HPWL calculation.

---

## Problem 2: Incremental Placement Algorithm

### Algorithm Approach

A greedy iterative improvement algorithm optimized for runtime performance. The scoring metric is **HPWL² × Time**, so minimizing execution time is prioritized while achieving HPWL reduction.

### Algorithm Steps

1. **Build Cache Structures**:
   - Index all instances and cache their center coordinates
   - Build net data structures with fixed pin bounds (using center points) and instance indices
   - Group instances by cell type and orientation (only identical cells with same orientation can be swapped)

2. **Greedy Iteration Loop**:
   - For each iteration:
     a. Evaluate all unlocked instance pairs within each group
     b. For each candidate pair:
        - Calculate HPWL change (delta) using cached coordinates (no OA API calls)
        - Track the best (most negative) delta found
     c. If a beneficial swap is found (negative delta):
        - Swap the two instances by exchanging their cached coordinates
        - Apply swap in OA database using `setTransform()`
        - Lock both swapped instances and all instances connected to affected nets
     d. Repeat until no beneficial swaps are found

### Optimization Strategies

- **Cached computation**: All HPWL calculations use cached coordinates, avoiding expensive OA API calls during swap evaluation
- **Incremental updates**: Only affected nets are recalculated when evaluating swaps
- **Locking strategy**: After a swap, the swapped instances and all instances connected to affected nets are locked to prevent redundant evaluations
- **Early termination**: Algorithm stops when no beneficial swaps are found

### Swap Implementation

- Extract current transforms (position + orientation) of both instances
- Create new transforms with swapped positions but maintaining original orientations
- Apply transforms using OpenAccess `setTransform()` API
- Update cached coordinates to match database state

### Results

- **Original HPWL**: 5,599,400 DBU
- **Final HPWL**: [Value after optimization]
- **HPWL Reduction**: [Improvement achieved]
- **Number of swaps**: [Count of swaps applied]
- **Execution time**: [Runtime in seconds]
- **Score (HPWL² × Time)**: [Final score metric]

### Key Design Decisions

1. **Cached data structures**: Pre-compute all instance centers and net connectivity to avoid OA API overhead during optimization
2. **Incremental delta calculation**: Only compute HPWL change for affected nets, not the entire design
3. **Locking mechanism**: Lock instances after swaps to prevent redundant pair evaluations while maintaining exploration of different swap opportunities
4. **Consistent HPWL calculation**: Uses the same center-point approach as Problem 1 for accurate comparison

The algorithm balances HPWL improvement with fast execution time to achieve a competitive overall score.

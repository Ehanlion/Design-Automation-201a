# Lab 3 Part 2: Incremental Placement Algorithms

This guide covers the four placement algorithms for **EE 201A Lab 3 Problem 2** and what you need to implement each one.

---

## Problem Overview

**Goal:** Reduce total Half-Perimeter Wirelength (HPWL) of a placed design using **cell swapping only**.

**Constraints (must enforce):**
1. Swaps only between **identical cells** (same master design/cell type)
2. Swaps only between instances with **identical orientation**
3. No movement operation other than **direct location swap** (no shifting, rotating, etc.)
4. **Fixed** and **locked** instances are not movable

**Score metric:** `HPWL² × runtime` — **lower is better**

---

## Required Output Format

Your program must print:

```
Problem 1 -- Total wirelength of original design: <original HPWL> DBU
Problem 2 -- Total wirelength AFTER my incremental placement algorithm:  <final HPWL> DBU
Problem 2 -- Total number of swaps used:  <N>
Problem 2 -- Time taken:  <seconds> sec
```

---

## Algorithm Summaries

| Algorithm | HPWL Improvement | Runtime | Complexity | Best For |
|-----------|------------------|---------|------------|----------|
| [Greedy Pairwise](part2_greedy_pairwise.md) | Moderate | Slow | O(n²) | Baseline / simple designs |
| [Window-Based](part2_window_based_local.md) | Moderate | Fast | O(n·window) | Runtime optimization |
| [Centroid Swapping](part2_centroid_swapping.md) | High | Moderate | O(n·nets) | Large HPWL reductions |
| [Simulated Annealing](part2_simulated_annealing.md) | Highest | Very slow | Variable | Best absolute HPWL |

---

## Prerequisites (All Algorithms)

Before implementing any algorithm, you need:

1. **OpenAccess design** loaded and opened in **append mode** (`'a'`) to allow `setOrigin()`
2. **Instance data:** `oaInst`, master, orientation, origin, bounding box centers
3. **Net data:** nets, instance connections, primary I/O pin positions
4. **HPWL computation:** per-net HPWL using instance centers + term pin positions
5. **Legal swap check:** same master + same orientation + both movable
6. **Affected nets:** only recompute HPWL for nets connected to swapped instances (incremental update)

---

## Algorithm Guides

- **[Greedy Pairwise Exchange](part2_greedy_pairwise.md)** — Baseline, easiest to implement
- **[Window-Based Local Optimization](part2_window_based_local.md)** — Faster runtime via spatial binning
- **[Net-Centric Centroid Swapping](part2_centroid_swapping.md)** — Smarter candidate selection
- **[Simulated Annealing](part2_simulated_annealing.md)** — Best quality, hardest to tune

---

## Quick Reference: File Structure

```
lab3/
├── lab3.cpp              # Main lab skeleton (Problem 1 + 2 placeholder)
├── lab3_p2_greedy_lock.cpp   # Example: Greedy with locking
├── Makefile              # Build with PROG=lab3 or PROG=lab3_p2_greedy_lock
├── scripts/
│   ├── setup_oa_database.sh
│   └── compile_run_p2_greedy_lock.sh
└── docs/
    ├── README.md              # This file
    ├── part2_greedy_pairwise.md
    ├── part2_window_based_local.md
    ├── part2_centroid_swapping.md
    ├── part2_simulated_annealing.md
    └── placement_types.md
```

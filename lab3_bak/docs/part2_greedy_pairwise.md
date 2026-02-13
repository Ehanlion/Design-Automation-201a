# Approach 1: Greedy Pairwise Exchange (Baseline)

The baseline approach — direct implementation of the "easy way" suggested in the lab manual, focusing on immediate improvement.

---

## Logic

Iterate through instances and identify pairs that are **identical in type and orientation**. For each pair, compute the change in HPWL if they were swapped. If HPWL decreases, commit the swap; otherwise, skip.

---

## Execution Steps

1. **Select a candidate pair** (A, B) such that:
   - Same master/cell type
   - Same orientation
   - Both movable (not fixed/locked)

2. **Compute current HPWL** of all nets connected to A and B.

3. **Temporarily swap** their coordinates and recompute HPWL for those same nets.

4. **If new HPWL < old HPWL:** commit the swap, update data structures, and optionally restart the sweep.

5. **Repeat** until no improving swap is found (or until a limit).

---

## What You Need to Implement

| Component | Description |
|-----------|-------------|
| **Instance iteration** | Double loop over all instance pairs `(i, j)` with `j > i` |
| **Legal swap check** | `sameMaster(A,B) && sameOrient(A,B) && movable(A) && movable(B)` |
| **Affected nets** | Union of nets connected to A and B — only these change HPWL |
| **Delta HPWL** | `sum(HPWL_old over affected nets) - sum(HPWL_new over affected nets)` |
| **Swap commit** | `A.setOrigin(B.origin)`, `B.setOrigin(A.origin)`, update centers and net HPWL cache |

---

## Pros and Cons

| Pros | Cons |
|------|------|
| Simple to implement | Can be slow — O(n²) pairs |
| Guaranteed to never increase HPWL | No exploration of bad moves |
| Uses same core as other algorithms | May get stuck in local minima |

---

## Speed Optimization

- Limit search to pairs that are **physically close** (within some radius)
- Limit search to pairs that **share nets** (common connections)
- Use **restart-on-swap** or **passes** to control when to stop

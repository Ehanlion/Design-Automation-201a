# Approach 2: Window-Based Local Optimization

Limits the search area to maintain fast execution, based on stochastic and detailed placement concepts.

---

## Logic

Instead of checking all possible pairs across the entire chip, define a sliding **window** or **grid**. Only attempt swaps between valid identical cells **within that window**.

---

## Execution Steps

1. **Divide the placement area** into a grid of bins (e.g., 2D array of bins).

2. **Assign each instance** to the bin containing its center.

3. **For each bin:**
   - Collect instances in that bin (and optionally neighboring bins for boundary cases)
   - Group by `(master, orientation)` — only identical cells can swap
   - Run Greedy Pairwise Exchange **only within that group**

4. **Slide the window** — either shift bins or use multiple passes with different grids.

5. **Repeat** until no improvement or a limit is reached.

---

## What You Need to Implement

| Component | Description |
|-----------|-------------|
| **Grid/bins** | 2D spatial hash: `bin_id = f(centerX, centerY)` based on chip bounds |
| **Bin assignment** | Map each instance to its bin; maintain `bin_id → list of instance indices` |
| **Per-bin pairing** | Within each bin, only iterate pairs of same-type, same-orient instances |
| **Greedy core** | Same swap logic as Approach 1: compute ΔHPWL, commit if improving |
| **Window size** | Tune bin dimensions — smaller bins = faster but more local |

---

## Pros and Cons

| Pros | Cons |
|------|------|
| Much faster than full O(n²) | May miss globally beneficial swaps |
| Manages candidate pairs well | Effective when global order is already good |
| Good for detailed placement | Less effective if placement is very poor |

---

## Implementation Tips

- Use `floor((x - minX) / binWidth)` and similar for Y to get bin indices.
- Optionally overlap windows or use multiple grid resolutions.
- Consider bin size ~2–4× average cell width for reasonable locality.

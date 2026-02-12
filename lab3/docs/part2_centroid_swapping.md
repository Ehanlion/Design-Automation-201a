# Approach 3: Net-Centric "Centroid" Swapping

Uses force-directed / analytic placement ideas to identify where a cell "wants" to be, then finds a swap partner near that ideal position.

---

## Logic

A cell should ideally be placed at the **centroid** (average position) of all the pins it connects to. For a cell A that is far from its centroid, look for an identical cell B that is currently near A’s ideal position. Swapping A and B may reduce HPWL.

---

## Execution Steps

1. **For each movable cell A:**
   - Get all nets connected to A
   - Compute centroid of connected pins:  
     `centroid = (avg x of pin positions, avg y of pin positions)`

2. **Find candidate B** such that:
   - Same master and orientation as A
   - B is **near** the centroid of A (within a radius or in the same bin)
   - B is movable

3. **Evaluate swap:** compute ΔHPWL if A and B swap.

4. **If ΔHPWL > 0:** commit the swap.

5. **Repeat** for all cells (or until no improvement).

---

## What You Need to Implement

| Component | Description |
|-----------|-------------|
| **Centroid computation** | For each instance, collect pin positions from its nets; average x and y |
| **Pin positions** | Instance center for inst pins; term center for I/O pins |
| **Spatial lookup** | Find instances near a given (x,y) — use bins or k-d tree |
| **Candidate selection** | Among instances near centroid, pick same-type, same-orient, movable |
| **Swap evaluation** | Same as Greedy: compute ΔHPWL for affected nets, commit if improving |

---

## Pros and Cons

| Pros | Cons |
|------|------|
| More "intelligent" than random swapping | More complex to implement |
| Targets cells far from optimal | Requires spatial data structure |
| Can achieve larger HPWL drops per swap | Centroid is only an approximation |

---

## Implementation Tips

- Use the same bins as in Window-Based for spatial lookup.
- For each cell, compute centroid once per pass; cache if needed.
- Search radius around centroid: start with ~1–2× average cell size.

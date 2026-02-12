# Placement Types and Constraints (Lab 3 Part 2)

Understanding cell placement status and swap legality is essential for implementing any incremental placement algorithm.

---

## OpenAccess Placement Status

Each instance has a `placementStatus` that affects whether it can be moved:

| Status | Meaning | Swappable? |
|--------|---------|------------|
| `oacNone` | Unplaced | No (not in legal placement) |
| `oacPlaced` | Placed, movable | **Yes** |
| `oacFixed` | Fixed in place | **No** |
| `oacLocked` | Locked (e.g., pre-placed macro) | **No** |

**Check before swapping:** both instances must have `placementStatus != oacFixed` and `!= oacLocked`.

---

## Swap Legality Rules

A swap between instances A and B is **legal** only if:

1. **Same master design** — `A->getMaster() == B->getMaster()` (or same cell name)
2. **Same orientation** — `A->getOrient() == B->getOrient()`
3. **Both movable** — neither fixed nor locked

---

## Orientation

Instances can have different orientations (R0, R90, R180, R270, MX, MY, etc.). Swapping cells with different orientations would change the layout in unintended ways, so the lab constrains swaps to **identical orientation** only.

---

## Wirelength Metric: HPWL

**Half-Perimeter Wirelength** for a net:

```
HPWL = (x_max - x_min) + (y_max - y_min)
```

Where the min/max are taken over all pin positions (instance centers and primary I/O pin centers).

- **Instance pin:** use bounding box center of the instance
- **Primary I/O pin:** use center of the pin figure’s bounding box

---

## Score Metric (Lab 3)

```
Score = HPWL² × runtime (seconds)
```

**Lower is better.** This favors both low wirelength and fast execution.

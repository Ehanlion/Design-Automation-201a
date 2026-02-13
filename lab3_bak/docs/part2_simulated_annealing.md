# Approach 4: Restricted Simulated Annealing (Hill-Climbing)

Adapts Simulated Annealing to the lab’s swap-only constraint. Allows occasional "bad" swaps early on to escape local HPWL minima.

---

## Logic

Use randomized moves to explore the solution space. Accept improving swaps always; accept worsening swaps with a probability that decreases over time (cooling).

---

## Execution Steps

1. **Initialize temperature** \( T = T_0 \) (e.g., based on initial HPWL or a fixed value).

2. **Randomly pick** two identical cells (A, B) with the same orientation.

3. **Compute** \( \Delta\text{HPWL} = \text{HPWL}_{\text{new}} - \text{HPWL}_{\text{old}} \).

4. **Acceptance rule:**
   - If \( \Delta\text{HPWL} < 0 \): always accept the swap.
   - If \( \Delta\text{HPWL} \geq 0 \): accept with probability  
     \( P = e^{-\Delta\text{HPWL} / T} \).

5. **Cooling:** decrease \( T \) over time (e.g., \( T \leftarrow \alpha T \) per iteration or per epoch).

6. **Stop** when \( T \) is small or a time/iteration limit is reached.

---

## What You Need to Implement

| Component | Description |
|-----------|-------------|
| **Random pair selection** | From same-type, same-orient, movable pairs — use uniform random |
| **Cooling schedule** | Geometric: `T *= alpha` (e.g., α = 0.95–0.99) |
| **Acceptance** | `if (delta < 0 || drand48() < exp(-delta/T))` then accept |
| **Initial temperature** | Often \( T_0 \propto \text{initial HPWL} \) or a heuristic |
| **Stopping** | Time limit, iteration limit, or \( T < T_{\min} \) |

---

## Pros and Cons

| Pros | Cons |
|------|------|
| Can escape local minima | Very slow |
| May find better global optimum | Requires parameter tuning |
| Well-studied algorithm | Score metric (HPWL² × time) penalizes long runtime |

---

## Parameter Tuning

| Parameter | Typical range | Effect |
|-----------|---------------|--------|
| \( T_0 \) | 0.01–0.1 × initial HPWL | Higher = more random exploration |
| \( \alpha \) | 0.95–0.999 | Slower cooling = more exploration |
| Iterations per T | 10–100× n | More tries before cooling |

**Note:** The lab score is `HPWL² × runtime`. Simulated Annealing tends to be slow, so it may hurt your score unless you strictly limit iterations or time.

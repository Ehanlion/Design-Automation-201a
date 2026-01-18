# EE 201A Lab 1 - Problem 2 Report
## Average Fanout Computation

**Student:** Ethan Owen  
**UID:** 905452983  
**Date:** January 17, 2026

---

## Objective
Write a C++ routine using the OpenAccess API to compute the overall average fanout in the `s1196_bench` design. Generate a fanout distribution plot and document the methodology and assumptions.

---

## Definition of Fanout

**Fanout** = Number of outputs (load points) connected to a net.

In the context of OpenAccess:
- Fanout of a net is the count of **output connections** on that net
- This includes:
  - **oaInstTerm** - Instance terminals (pins on standard cell instances)
  - **oaTerm** - Primary I/O terminals (top-level design pins)

According to the TA guidance: "Roughly, fanout of a net is the number of its outputs."

---

## Methodology

### Approach
1. **Iterate through all nets** in the design's top block
2. **For each net, count connections:**
   - Count all `oaInstTerm` objects connected to the net
   - Count all `oaTerm` objects connected to the net
   - Sum = fanout for that net
3. **Filter special nets** (optional, based on requirements):
   - Power nets (VDD, VCC, etc.)
   - Ground nets (VSS, GND, etc.)
   - Clock nets (CLK, clock, etc.)
4. **Accumulate statistics:**
   - Total fanout sum across all nets
   - Net count
   - Calculate average = total fanout / net count
5. **Generate fanout distribution** for plotting

### OpenAccess API Hierarchy

The hint provided in the lab instructions:
```
oaBlock -> (oaIter) -> oaNet -> (oaInstTerm, oaTerm)
```

**Implementation flow:**
```cpp
oaBlock *block = design->getTopBlock();
oaIter<oaNet> netIterator(block->getNets());

while (oaNet *net = netIterator.getNext()) {
    int fanout = 0;
    
    // Count instance terminals
    oaIter<oaInstTerm> instTermIter(net->getInstTerms());
    while (oaInstTerm *instTerm = instTermIter.getNext()) {
        fanout++;
    }
    
    // Count primary terminals
    oaIter<oaTerm> termIter(net->getTerms());
    while (oaTerm *term = termIter.getNext()) {
        fanout++;
    }
    
    // Store fanout value
    fanoutArray.push_back(fanout);
}
```

---

## Assumptions

1. **Fanout Definition:** Counting all connections (both instance terminals and primary terminals) as outputs. This is a simplified definition suitable for post-routing analysis.

2. **Net Filtering:** 
   - **Option A:** Include all nets (including power/ground/clock) for a complete analysis
   - **Option B:** Exclude special nets if they skew the distribution
   - Document which approach was taken in the final report

3. **Empty Nets:** Nets with zero fanout are included in the count (they may represent unconnected signals or optimization targets).

4. **Bidirectional Pins:** No distinction made between input/output/inout terminals - all connections count toward fanout.

5. **Post-Routing Design:** This analysis assumes the design is already placed and routed (which it is, based on the input DEF file).

---

## Implementation Details

### Code Structure

**File:** `lab1.cpp`  
**Section:** Problem 2 (lines ~253-293)

**Key variables:**
- `total_net` - Count of nets processed
- `fanout` - Fanout for current net
- `fanoutArray` - Vector storing all fanout values (for distribution plot)
- `totalFanout` - Sum of all fanout values

**Key iterators:**
- `oaIter<oaNet>` - Iterate through all nets
- `oaIter<oaInstTerm>` - Iterate through instance terminals on a net
- `oaIter<oaTerm>` - Iterate through primary terminals on a net

### Special Net Filtering (Optional)

If filtering is desired, check net names:
```cpp
oaString netName;
net->getName(ns, netName);

// Skip power/ground nets
if (netName == "VDD" || netName == "VSS" || 
    netName == "GND" || netName == "VCC") {
    continue;
}

// Skip clock nets
if (netName.contains("CLK") || netName.contains("clock")) {
    continue;
}
```

---

## Compilation and Execution

### Prerequisites
1. OpenAccess environment properly set up (see `lab1_setup`)
2. Design imported into DesignLib (run Problem 1 import first)

### Compilation Script
```bash
./scripts/problem2-compile.sh
```

**Or manually:**
```bash
source lab1_setup
make lab1 CCPATH=g++
```

### Execution
```bash
./lab1
```

### Expected Output
```
The design is created and opened in 'write' mode.
	The library name for this design is : DesignLib
	The cell name for this design is : s1196_bench
	The view name for this design is : layout
The following nets exist in this design.
	[... list of nets ...]

----- Ethan Owen: Problem 2 -----
Problem 2 -- Average fanout [VALUE]
```

---

## Data Collection for Plot

### Fanout Distribution Data
The program should output or save:
1. **Fanout values for each net** (can be saved to a file for MATLAB/Python processing)
2. **Summary statistics:**
   - Average fanout
   - Minimum fanout
   - Maximum fanout
   - Standard deviation (optional)

### Generating the Plot

**Using MATLAB:**
```matlab
% Load fanout data
fanout_data = load('fanout_data.txt');

% Create histogram
figure;
histogram(fanout_data, 'BinMethod', 'integers');
xlabel('Fanout');
ylabel('Number of Nets');
title('Fanout Distribution for s1196\_bench Design');
grid on;

% Save figure
saveas(gcf, 'fanout_distribution.png');
```

**Using Python:**
```python
import numpy as np
import matplotlib.pyplot as plt

# Load fanout data
fanout_data = np.loadtxt('fanout_data.txt')

# Create histogram
plt.figure(figsize=(10, 6))
plt.hist(fanout_data, bins=range(int(max(fanout_data))+2), 
         edgecolor='black', alpha=0.7)
plt.xlabel('Fanout')
plt.ylabel('Number of Nets')
plt.title('Fanout Distribution for s1196_bench Design')
plt.grid(True, alpha=0.3)
plt.savefig('fanout_distribution.png', dpi=300)
plt.show()

# Print statistics
print(f"Average Fanout: {np.mean(fanout_data):.2f}")
print(f"Min Fanout: {int(min(fanout_data))}")
print(f"Max Fanout: {int(max(fanout_data))}")
print(f"Std Dev: {np.std(fanout_data):.2f}")
```

---

## Common Issues and Solutions

### Issue 1: Undefined Variables During Compilation
**Error:** `'fanout' was not declared in this scope`

**Solution:** 
- Declare all variables before use
- Make sure the Problem 2 code section is complete
- If testing Problem 2 before Problem 3, comment out the Problem 3 section

### Issue 2: Compiler Not Found
**Error:** `make: /grid/common/pkgs/gcc/v4.1.2p2/bin/g++: Command not found`

**Solution:** Use `make lab1 CCPATH=g++` to use the system default g++

### Issue 3: Empty or Missing Block
**Error:** Program crashes or prints "There is no block in this design"

**Solution:** 
- Ensure DesignLib is properly imported
- The code includes a check to create a block if missing:
  ```cpp
  if (!block) {
      block = oaBlock::create(design);
  }
  ```

### Issue 4: OpenAccess Tools Not Found
**Error:** `lef2oa: command not found` or similar

**Solution:**
- Make sure `lab1_setup` is sourced
- Verify OA_HOME, LD_LIBRARY_PATH, and PATH are set correctly
- See `_Training/Training-Lab1.md` for environment setup details

---

## References

1. OpenAccess API Documentation
2. Lab 1 Instructions (Lab 1.pdf)
3. Training notes (_Training/Training-Lab1.md)
4. Example code: `/w/class.1/ee/ee201o/ee201ota/oa/examples/`

---

## Results Summary

**Average Fanout:** [TO BE FILLED AFTER RUNNING]

**Fanout Distribution:**
- [Plot will be inserted here]
- Discussion of distribution characteristics
- Analysis of any outliers or interesting patterns

---

## Conclusion

This problem demonstrates how to traverse OpenAccess design databases to extract connectivity information. The fanout metric is fundamental for:
- Timing analysis (high fanout can cause delays)
- Power analysis (fanout affects switching power)
- Signal integrity (high fanout may need buffering)
- Design quality assessment

The implementation uses standard OpenAccess iterators to walk the net hierarchy and count connections, providing a foundation for more complex design analysis tools.

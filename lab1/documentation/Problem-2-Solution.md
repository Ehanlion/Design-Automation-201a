# Lab 1 - Problem 2: Average Fanout Summary

**To compile and run (if not already done):**
```bash
./scripts/problem2-compile.sh
./lab1
```

**Or manually:**
```bash
source lab1_setup
make lab1 CCPATH=g++
./lab1
```

---

## Quick Reference

**Definition:** Fanout = number of outputs (connections) on a net

**API Hierarchy:** 
```
oaBlock -> oaNet -> (oaInstTerm + oaTerm)
```

**Key Code Pattern:**
```cpp
oaBlock *block = design->getTopBlock();
oaIter<oaNet> netIterator(block->getNets());
vector<int> fanoutArray;

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
    
    fanoutArray.push_back(fanout);
}

// Calculate average
double avgFanout = accumulate(fanoutArray.begin(), 
                               fanoutArray.end(), 0.0) 
                   / fanoutArray.size();
```

---

## Results

**Average Fanout:** [VALUE AFTER RUNNING]

**Distribution Plot:** [fanout_distribution.png]

---

## Notes

- **TA Guidance:** "Fanout of a net is the number of its outputs"
- **Filtering:** Consider excluding VDD/VSS/CLK nets
- **Compilation:** Use `CCPATH=g++` if default compiler fails
- **Data Export:** Save fanout values to file for MATLAB/Python plotting

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Undefined variables | Complete Problem 2 code or comment out |
| Compiler not found | Use `make lab1 CCPATH=g++` |
| No DesignLib | Run `./scripts/problem1-import.sh` first |
| OA tools not found | Source `lab1_setup` |

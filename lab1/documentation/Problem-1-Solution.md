# Lab 1 - Problem 1: Design Import Summary

**To import the design (if not already done):**
```bash
./import.sh
```

**Files:**
- *LEF Import:* NangateOpenCellLibrary.lef -> NangateLib (163 cells)
- *Verilog Import:* s1196_postrouting.v -> DesignLib/s1196_bench/layout
- *DEF Import:* s1196_postrouting.def -> Added physical layout

**Commands Used (For Your Report)**

```bash
# Setup environment
source lab1_setup

# Import LEF file (reference library)
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef

# Import Verilog netlist (logical design)
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v

# Import DEF file (physical layout)
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib
```
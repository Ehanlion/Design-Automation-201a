# EE 201A Lab 1 - Problem 1 Report
## Import Design Information into OpenAccess Database

**Student:** Ethan Owen  
**UID:** 905452983  
**Date:** January 17, 2026

---

## Objective
Import design information from Verilog and DEF files, along with library information from LEF files, into a single OpenAccess (OA) database named `DesignLib`.

---

## Files Used

### Input Files:
1. **NangateOpenCellLibrary.lef** - Library Exchange Format file containing standard cell definitions
2. **s1196_postrouting.v** - Verilog netlist describing the logical connectivity of design `s1196_bench`
3. **s1196_postrouting.def** - Design Exchange Format file containing physical layout information

### Design Information:
- **Design Name:** s1196_bench
- **View:** layout
- **View Type:** maskLayout (physical layout)

---

## Import Procedure

The import process follows the standard LEF/DEF/Verilog to OpenAccess translation flow using three main commands:

### Step 1: Import LEF File (Create Reference Library)
**Command:**
```bash
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef
```

**Purpose:** Creates the reference library `NangateLib` containing all standard cell definitions from the LEF file. This library contains 163 standard cells (gates, flip-flops, buffers, etc.).

**Result:** 
- Created `NangateLib` directory with all reference cells
- Status: 0 errors, 1 warning (auto-created lib.defs file)

---

### Step 2: Import Verilog Netlist (Create Design Library)
**Command:**
```bash
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v
```

**Purpose:** Imports the Verilog netlist to create the design library `DesignLib`. This establishes the logical connectivity of the design by referencing cells from `NangateLib`.

**Parameters:**
- `-lib DesignLib`: Target library name for the design
- `-refLibs NangateLib`: Reference library containing cell definitions
- `-view layout`: View name for the design
- `-viewType maskLayout`: Specifies this is a physical layout view
- `-verilog s1196_postrouting.v`: Input Verilog file

**Result:**
- Created `DesignLib/s1196_bench/layout`
- Detected top module: `s1196_bench`
- Status: 0 errors, 0 warnings

---

### Step 3: Import DEF File (Add Physical Layout Information)
**Command:**
```bash
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib
```

**Purpose:** Annotates the logical design with physical implementation details (component placement, routing, die area, etc.) from the DEF file.

**Parameters:**
- `-lib DesignLib`: Target library containing the design
- `-cell s1196_bench`: Specific cell to annotate
- `-view layout`: View to annotate
- `-def s1196_postrouting.def`: Input DEF file with physical data
- `-refLibs NangateLib`: Reference library for cell lookups

**Result:**
- Successfully annotated `DesignLib/s1196_bench/layout` with physical data
- Status: 0 errors, 0 warnings

---

## Results and Verification

### Created Directory Structure:
```
lab1/
├── DesignLib/              # Design library
│   ├── data.dm
│   └── s1196_bench/        # Design cell
│       └── layout/         # Layout view
│           ├── layout.oa   # OpenAccess database file
│           └── master.tag
├── NangateLib/             # Reference library (163 cells)
│   ├── AND2_X1/
│   ├── AND2_X2/
│   ├── DFF_X1/
│   ├── NAND2_X1/
│   └── ... (160+ more cells)
└── lib.defs                # Library definitions file
```

### Library Definitions File (lib.defs):
```
DEFINE NangateLib NangateLib
ASSIGN NangateLib writePath NangateLib
ASSIGN NangateLib libMode shared
DEFINE DesignLib DesignLib
ASSIGN DesignLib writePath DesignLib
ASSIGN DesignLib libMode shared
```

### Import Summary:
- LEF import: **Successful** (3.31 seconds, 0 errors)
- Verilog import: **Successful** (0.24 seconds, 0 errors)
- DEF import: **Successful** (0.17 seconds, 0 errors)
- **Total import time:** ~3.7 seconds
- **Reference cells imported:** 163 standard cells
- **Design imported:** s1196_bench/layout

---

## Command Summary

**Complete command sequence for Problem 1:**

```bash
# Setup environment
source lab1_setup

# Import LEF (reference library)
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef

# Import Verilog (logical design)
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v

# Import DEF (physical layout)
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib
```

**Or simply run the automated script:**
```bash
./import_design.sh
```

---

## Notes

1. **Simplified Commands:** As instructed, used the simplest version of each command without unnecessary options.

2. **No Technology Library Specification:** Unlike the example in the lab instructions that used `-techLib foundryLib`, we didn't need this option since we only have one LEF file. The technology data is automatically extracted from the LEF during import.

3. **No SPEF Import:** Parasitic extraction data (SPEF) is optional and not required for this lab.

4. **Order Matters:** The import sequence must be followed:
   - LEF first (creates reference library)
   - Verilog second (creates logical design using references)
   - DEF last (adds physical data to existing design)

5. **Error Handling:** The automation script includes error checking after each step to catch any import failures.

---

## Conclusion

Successfully imported the s1196_bench design into OpenAccess database `DesignLib` with all reference cells in `NangateLib`. The design is now ready for analysis in Problems 2 and 3 of the lab.

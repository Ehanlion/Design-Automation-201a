# Lab 1 Quick Steps (Parts 1-3)

Use this as a short, practical checklist while you work through the lab.

---

## Problem 1 -- Import Design into OpenAccess [COMPLETE]

**What this problem is asking for**
Import LEF/DEF/Verilog files into an OpenAccess database called `DesignLib`. This creates the OA database that you'll use for Problems 2 and 3.

**Files to import:**
- `NangateOpenCellLibrary.lef` - Standard cell library definitions
- `s1196_postrouting.v` - Verilog netlist (logical connectivity)
- `s1196_postrouting.def` - Physical layout information

**[SOLUTION] - Run the import script:**
```bash
./import_design.sh
```

**Or run manually (3 commands):**
```bash
source lab1_setup  # Set up OA environment

# 1. Import LEF -> creates NangateLib (163 standard cells)
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef

# 2. Import Verilog -> creates DesignLib with logical design
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v

# 3. Import DEF -> adds physical layout to design
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib
```

**Verification:**
- [OK] `DesignLib/` directory exists with `s1196_bench/layout/`
- [OK] `NangateLib/` directory exists with 163 reference cells
- [OK] All commands report **0 errors**

**Report:** See `PROBLEM1_REPORT.md` for detailed documentation.

---

## Part 1 -- OpenAccess warm-up (design open + net listing)
**What this part is asking for**
Show you can open the OA design database, read basic design metadata, create a couple of nets, save them, and iterate/print all nets in the design.

**Step-by-step**
1. **Set up OA environment**  
   Ensure OA tools are on your path. If needed, add and source the lines in `lab1_setup`:
   `OA_HOME`, `LD_LIBRARY_PATH`, `PATH`.
2. **Build**  
   Run `make lab1` (if compiler path fails, use `make lab1 CCPATH=g++`).
3. **Open the design**  
   In `lab1.cpp`, confirm the library/cell/view names and open the design in read mode.
4. **Print design identifiers**  
   Print the library, cell, and view names (already in `printDesignNames()`).
5. **Create nets**  
   Create two nets named `Hello` and `World` in the top block.
6. **Save**  
   Save the created nets to the design.
7. **Iterate nets**  
   Iterate through all nets in the block and print their names (already in `printNets()`).

**Checks**
- Program runs without OA errors.
- Output includes library/cell/view names and a full net list that includes `Hello` and `World`.

## Part 2 -- Average fanout
**What this part is asking for**
Compute the **average fanout** across nets in the design. Fanout is the number of **outputs** on a net (in practice here: count connected instance terminals and primary terminals). Exclude special nets (power/ground/clock) if the lab specifies.

**Step-by-step**
1. **Iterate nets in the top block**.
2. **For each net, count connections**  
   - Iterate `oaInstTerm` connected to the net.  
   - Iterate `oaTerm` (top-level terminals) connected to the net.  
   - Fanout ~= number of outputs on the net (see TA note: "fanout = number of its outputs").
3. **Filter special nets**  
   Skip power/ground/clock nets by name if required (e.g., `VDD`, `VSS`, `CLK`, etc.).
4. **Accumulate and average**  
   Sum fanouts and divide by number of valid nets.
5. **Print the average**.

**Checks**
- No compile errors from undefined variables (complete Part 2 code before building).
- Output: `Problem 2 -- Average fanout <value>`.

## Part 3 -- Average wirelength
**What this part is asking for**
Compute the **average wirelength** of nets in the layout by traversing OA geometry. This usually means summing geometric lengths from pin figures associated with nets and averaging across nets (excluding special nets if required).

**Step-by-step**
1. **Iterate nets in the top block**.
2. **Walk pin geometry**  
   For each net, traverse the OA hierarchy:
   `oaTerm` -> `oaPin` -> `oaPinFig` (inherits from `oaFig`) to access geometry.
3. **Compute net wirelength**  
   Use the pin figure geometry (boxes/paths) to compute length for that net.
4. **Filter special nets**  
   Skip power/ground/clock nets if required.
5. **Average and print**  
   Sum net lengths and divide by number of valid nets.

**Checks**
- Output: `Problem 3 -- Average wirelength <value>`.
- Code handles designs with missing blocks by creating a block before use.

## Common pitfalls
- **Compiler path error**: use `make lab1 CCPATH=g++`.
- **Undefined variables**: Part 2/3 placeholders must be implemented before `make`.
- **OA tools not found**: ensure `OA_HOME` and paths are exported (see `lab1_setup`).

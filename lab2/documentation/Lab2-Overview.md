# Lab 2: Logic Synthesis with Cadence Genus - Assignment Overview

**Course:** UCLA EE 201A -- VLSI Design Automation – Winter 2026  
**TA:** Dedeepyo Ray  
**Due Date:** Tuesday, Feb 3, 2026 at 11:59:59 P.M. Pacific Daylight Time (PDT)

## Important Notes

- **Work Location:** Run experiments on SEASnet server `eeapps.seas.ucla.edu`
- **Backup:** You are responsible for backing up your code & data
- **Files Location:** All necessary files in `/w/class.1/ee/ee201o/ee201ot2/2024_labs/lab2`
- **Setup:** Use `new_csh_ee201a_setup` to set up your shell environment
- **Modifications:** Only modify the skeleton Tcl script (`lab2_skeleton.tcl`). Do NOT modify:
  - Input design (Verilog file `s15850.v`)
  - Nangate library (Liberty file `NangateOpenCellLibrary_typical.lib`)

---

## Problem 1: TCL Practice Problem (3 points)

### Objective
Get familiar with Tcl scripting, which is essential for Genus and Innovus. This is a pure Tcl practice problem (no Genus/Innovus involved).

### Deliverables
- **Report:** List results and attach screenshot of your Tcl script
- **Results File:** Fill in `results_submission.txt` with:
  - Average wirelength
  - Number of wires with length larger than 50

### Files Needed
- `wirelength.txt` - Contains wirelength data (one wirelength per line)

### Task Order
1. **Read the wirelength file**
   - Use Tcl file I/O commands to read `wirelength.txt`
   - Parse each line to extract wirelength values

2. **Calculate average wirelength**
   - Sum all wirelengths
   - Divide by total count
   - Store result

3. **Count wires with length > 50**
   - Iterate through wirelengths
   - Count entries where value > 50

4. **Create Tcl script**
   - Write a standalone Tcl script (e.g., `Lab2_1.tcl`)
   - Output results to console or file
   - Take screenshot for report

5. **Run script**
   ```bash
   tclsh Lab2_1.tcl
   ```

### Suggested Code Structure
```tcl
# Open and read wirelength.txt
set fp [open "wirelength.txt" r]
set wirelengths [list]
while {[gets $fp line] >= 0} {
    lappend wirelengths $line
}
close $fp

# Calculate average
set sum 0
set count [llength $wirelengths]
foreach wl $wirelengths {
    set sum [expr {$sum + $wl}]
}
set avg [expr {$sum / $count}]

# Count wires > 50
set count_over_50 0
foreach wl $wirelengths {
    if {$wl > 50} {
        incr count_over_50
    }
}

# Print results
puts "Average wirelength: $avg"
puts "Number of wires with length > 50: $count_over_50"
```

### Useful Resources
- Tcl Tutorial: https://www.tutorialspoint.com/tcl-tk/index.htm
- Tcl Manual: https://tcl.tk/man/tcl8.5/tutorial/tcltutorial.html

---

## Problem 2: Find Fastest Achievable Clock Frequency (5 points)

### Part A: Basic Clock Period Optimization (2 points)

### Objective
Find the best clock period that can be achieved without creating timing violations by synthesizing with different target clock period constraints.

### Deliverables
- **Report:** 
  - Fastest clock period achieved
  - Explanation: Why a clock period of X violates timing, while 0.97X passes timing
- **Results File:** Fill in `BEST_CLOCK_PERIOD` in `results_submission.txt`

### Files Needed
- `lab2_skeleton.tcl` - Skeleton Tcl script (modify this)
- `s15850.v` - RTL Verilog design file
- `NangateOpenCellLibrary_typical.lib` - Technology library

### Task Order
1. **Fill in skeleton script blanks**
   - Replace `???` placeholders:
     - `set_db library ???` → `NangateOpenCellLibrary_typical.lib`
     - `set hdl_files {???.v}` → `s15850.v`
     - `set DNAME ???` → `s15850` (or similar)
     - `set DESIGN ???` → Top-level module name from `s15850.v`
     - `set clkpin clkname` → Actual clock pin name from design

2. **Set base working directory**
   - Ensure output directory exists: `mkdir -p output`

3. **Initial synthesis with default clock period**
   - Start with clock period = 1000 ps (from skeleton)
   - Run synthesis: `genus < lab2_skeleton.tcl`
   - Check timing reports in `output/synth_report_timing.txt`

4. **Iterative clock period reduction**
   - Reduce clock period incrementally (e.g., 1000 → 900 → 800 → ...)
   - For each period:
     - Modify `set clk_period` in script
     - Run synthesis
     - Check for negative slack (timing violations)
   - Continue until you find the smallest period with no negative slack

5. **Fine-tune for minimum slack**
   - Once violations appear, try smaller increments (e.g., 10 ps steps)
   - Find the period where slack is minimized but still non-negative

### Suggested Code Modifications
```tcl
# Modify only this line in the constraints section:
set clk_period 1000  # Start here, then reduce iteratively

# After synthesis, check timing:
# Look at output/synth_report_timing.txt for slack values
# Negative slack = timing violation
```

### Key Commands
- Run synthesis: `genus < lab2_skeleton.tcl`
- Check timing: Review `output/synth_report_timing.txt`
- Look for: Negative slack values indicate timing violations

---

### Part B: Advanced Optimization (3 points)

### Objective
Improve the attainable clock speed further by adding, modifying, or removing commands in the Tcl script. Cannot change timing constraints other than clock period.

### Deliverables
- **Report:**
  - Fastest clock period achieved
  - List exactly which commands were added/modified/removed
  - Explain why chosen and what they achieve
- **Results File:** Fill in `NEW_BEST_CLOCK_PERIOD` in `results_submission.txt`
- **Submission Files:**
  - `Lastname-Firstname_UID_Username_Lab2_2B.tcl` (modified script)
  - `Lastname-Firstname_UID_Username_Lab2_2B.v` (synthesized Verilog output)

### Files Needed
- Copy of skeleton script from Part A (with all `???` filled)
- Same design and library files

### Task Order
1. **Start with Part A script**
   - Use the script that achieved best clock period in Part A

2. **Experiment with optimization commands**
   - Try optimizing "total negative slack"
   - Experiment with different "effort" levels in synthesis commands
   - Consider:
     - `syn_opt` with different effort levels
     - `set_db syn_generic_effort` (low/medium/high)
     - `set_db syn_map_effort` (low/medium/high)
     - `set_db syn_opt_effort` (low/medium/high)
     - Additional optimization passes

3. **Test each modification**
   - Run synthesis with each change
   - Compare clock period achieved
   - Keep modifications that improve results

4. **Document changes**
   - Keep track of all commands added/modified/removed
   - Note the rationale for each change

### Suggested Code Modifications
```tcl
# Try different effort levels:
set_db syn_generic_effort high
set_db syn_map_effort high
set_db syn_opt_effort high

# Or try multiple optimization passes:
syn_opt
syn_opt  # Run again

# Or optimize specifically for timing:
syn_opt -no_incr
```

### Hints
- Focus on optimizing "total negative slack"
- Try different synthesis effort levels
- Multiple optimization passes may help

### Grading
- 1 point: Trying and reporting answer (even if no improvement)
- 1 point: Any improvement over Part A
- 1 point: Fastest clock with no timing violations (competitive)

---

## Problem 3: Area vs. Delay Tradeoff and Power Optimization (5 points)

### Part A: Area vs. Delay Tradeoff (2 points)

### Objective
Run synthesis flow while increasing clock period constraint from smallest period in Problem 2B until slack in most critical path is no more than 200 ps. Plot area vs. clock period.

### Deliverables
- **Report:**
  - Plot: Total synthesized design area vs. clock period constraint
    - Y-axis: Area
    - X-axis: Clock period (delay)
  - At least 8 clock period values
  - Explanation of observed trend

### Files Needed
- Script from Problem 2B (or skeleton with modifications)

### Task Order
1. **Start with Problem 2B script**
   - Use script that achieved fastest clock in Problem 2B

2. **Set initial clock period**
   - Start with smallest period from Problem 2B

3. **Run synthesis for multiple clock periods**
   - Increase clock period incrementally (e.g., +50 ps or +100 ps steps)
   - For each period:
     - Modify `set clk_period` in script
     - Run synthesis
     - Record:
       - Clock period
       - Total area (from `output/synth_report_gates.txt` or `report_gates`)
       - Critical path slack (from `output/synth_report_timing.txt`)

4. **Continue until slack ≤ 200 ps**
   - Stop when critical path slack is no more than 200 ps
   - Ensure you have at least 8 data points

5. **Create plot**
   - Plot area (Y-axis) vs. clock period (X-axis)
   - Use plotting tool of choice (Python matplotlib, Excel, etc.)

6. **Analyze trend**
   - Describe relationship between area and clock period
   - Explain why this trend occurs

### Suggested Code Modifications
```tcl
# Modify clock period for each run:
set clk_period 800  # Start from Problem 2B result, then increase

# After synthesis, extract area:
# Check output/synth_report_gates.txt for total area
# Or use: report_gates
```

### Expected Trend
- As clock period increases (slower timing), area typically decreases
- Faster timing requires larger cells/buffers → more area
- Slower timing allows smaller cells → less area

---

### Part B: Power Optimization (3 points)

### Objective
Modify skeleton script to synthesize design and optimize for low power without violating timing constraints. Clock period must be 1000 ps, slew rate must be 100 ps.

### Deliverables
- **Report:**
  - Lowest total power achieved
  - Power breakdown: leakage/dynamic/total power
  - List exactly which commands were added/modified/removed
  - Explain why chosen and what they achieve
- **Results File:** Fill in power values in `results_submission.txt`:
  - `LOWEST_TOTAL_POWER`
  - `LOWEST_LEAKAGE_PWR`
  - `LOWEST_DYNAMIC_PWR`
- **Submission Files:**
  - `Lastname-Firstname_UID_Username_Lab2_3B.tcl` (modified script)
  - `Lastname-Firstname_UID_Username_Lab2_3B.v` (synthesized Verilog output)

### Files Needed
- Fresh copy of skeleton script (with `???` filled)
- Same design and library files

### Task Order
1. **Start with fresh skeleton script**
   - Fill in all `???` placeholders
   - Set clock period to 1000 ps (required)
   - Set slew rate to 100 ps (required)

2. **Add power constraints**
   - Try constraining max dynamic power
   - Try constraining leakage power
   - Experiment with power optimization commands

3. **Try retiming for minimum area**
   - Retiming can help reduce power by optimizing register placement

4. **Run synthesis and measure power**
   - Check `output/synth_report_power.txt` for:
     - Total power
     - Leakage power
     - Dynamic power

5. **Iterate and optimize**
   - Try different power constraint values
   - Try different optimization strategies
   - Keep track of best results

6. **Document final approach**
   - Record all commands added/modified/removed
   - Explain rationale

### Suggested Code Modifications
```tcl
# Set power constraints:
set_db max_dynamic_power 0.0  # Constrain dynamic power
set_db max_leakage_power 0.0  # Constrain leakage power

# Enable power optimization:
set_db syn_opt_power_effort high

# Try retiming:
set_db retiming true

# Or use power-aware synthesis:
syn_opt -power
```

### Hints
- Constrain max dynamic power and/or leakage power
- Try "retiming" for minimum area
- Power optimization may require multiple passes

### Grading
- 1 point: Trying and reporting answer (even if no improvement)
- 1 point: Any improvement in total power over skeleton script
- 1 point: Lowest power achieved (competitive)

---

## Submission Procedure

### Step 1: Prepare Submission Directory

Create a directory named:
```
Lastname-Firstname_UID_Username_Lab2/
```

Example: `Gottscho-Mark_203555232_gottscho_Lab2/`

### Step 2: Required Files (Exactly 7 files)

Inside the submission directory, include exactly these files:

1. **`Lastname-Firstname_UID_Username_results_submission.txt`**
   - Filled-in `results_submission.txt` with all results
   - **CRITICAL:** Required for autograding

2. **`Lastname-Firstname_UID_Username_Lab2_2B.tcl`**
   - Synthesis script for Problem 2B

3. **`Lastname-Firstname_UID_Username_Lab2_2B.v`**
   - Synthesized Verilog output by Genus for Problem 2B
   - Found in `output/` directory after running synthesis

4. **`Lastname-Firstname_UID_Username_Lab2_3B.tcl`**
   - Synthesis script for Problem 3B

5. **`Lastname-Firstname_UID_Username_Lab2_3B.v`**
   - Synthesized Verilog output by Genus for Problem 3B
   - Found in `output/` directory after running synthesis

6. **`Lastname-Firstname_UID_Username_Lab2_1.tcl`**
   - Tcl script for Problem 1

7. **`Lastname-Firstname_UID_Username_Lab2.pdf`**
   - Lab report (also upload to Gradescope)

### Step 3: Set Permissions

**IMPORTANT:** All files and directories must have full read and execute permissions for group and others:

```bash
chmod -R go+rx Lastname-Firstname_UID_Username_Lab2/
```

### Step 4: Create Tarball

Compress and archive the directory:

```bash
tar -czf Lastname-Firstname_UID_Username_Lab2_pinXXXX.tar.gz Lastname-Firstname_UID_Username_Lab2/
```

- Replace `XXXX` with a 4-digit numeric PIN of your choice
- PIN can be any 4-digit number (need not match Lab 1)

### Step 5: Set Tarball Permissions

```bash
chmod go+rx Lastname-Firstname_UID_Username_Lab2_pinXXXX.tar.gz
```

### Step 6: Submit Tarball

Copy tarball to submission directory:

```bash
cp Lastname-Firstname_UID_Username_Lab2_pinXXXX.tar.gz /w/class.1/ee/ee201o/ee201ot2/submission/lab2/
```

### Step 7: Upload Report to Gradescope

- Also upload your PDF report (`Lastname-Firstname_UID_Username_Lab2.pdf`) to Gradescope

### Important Submission Notes

- **Late submissions:** Not accepted. Submission directory permissions disabled at deadline
- **Incomplete work:** Submit whatever you completed. No submission = no points
- **Multiple submissions:** You can overwrite your submission before deadline. Only latest timestamp considered
- **File naming:** Use exact format provided. Duplicates with "v1, v2, new, newest" etc. will be ignored
- **Privacy:** Submission directory has read permissions disabled, so others cannot list contents
- **Verification:** You cannot list submission directory contents to verify - you can only write to it

### Submission Example

```bash
# Navigate to your working directory
cd /w/class/ee201a/your_username

# Create submission directory
mkdir Gottscho-Mark_203555232_gottscho_Lab2

# Copy required files
cp synth2b.tcl Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2_2B.tcl
cp synth3b.tcl Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2_3B.tcl
cp output/s15850_synth2b.v Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2_2B.v
cp output/s15850_synth3b.v Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2_3B.v
cp problem1.tcl Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2_1.tcl
cp results_submission.txt Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_results_submission.txt
cp report.pdf Gottscho-Mark_203555232_gottscho_Lab2/Gottscho-Mark_203555232_gottscho_Lab2.pdf

# Set permissions
chmod -R go+rx Gottscho-Mark_203555232_gottscho_Lab2/

# Create tarball
tar -czf Gottscho-Mark_203555232_gottscho_Lab2_pin0072.tar.gz Gottscho-Mark_203555232_gottscho_Lab2/

# Set tarball permissions
chmod go+rx Gottscho-Mark_203555232_gottscho_Lab2_pin0072.tar.gz

# Submit
cp Gottscho-Mark_203555232_gottscho_Lab2_pin0072.tar.gz /w/class.1/ee/ee201o/ee201ot2/submission/lab2/
```

---

## Summary

This lab covers:
1. **Tcl scripting basics** (Problem 1) - File I/O and numerical operations
2. **Clock period optimization** (Problem 2) - Finding fastest achievable frequency
3. **Area-delay tradeoffs** (Problem 3A) - Understanding design space
4. **Power optimization** (Problem 3B) - Low-power synthesis techniques

Key skills developed:
- Tcl scripting for EDA tools
- Logic synthesis with Cadence Genus
- Timing constraint optimization
- Power-aware design optimization
- Understanding area-delay-power tradeoffs

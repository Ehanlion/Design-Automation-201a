# Lab 2 Setup Complete ✓

**Date:** January 29, 2026  
**Status:** All scripts tested and working

## What Was Set Up

### 1. Problem 1: TCL Practice Script ✓
- **Script:** `tcl_scripts/lab2_problem1.tcl`
- **Execution:** `bash scripts/run_problem1.sh`
- **Status:** Working correctly
- **Results:**
  - Average wirelength: **47.39**
  - Wires with length > 50: **89**
  - Output saved to: `results/problem1_results.txt`

### 2. Genus Synthesis Script ✓
- **Script:** `tcl_scripts/lab2_skeleton.tcl`
- **Execution:** `bash scripts/run_synthesis.sh`
- **Status:** Working correctly (headless mode)
- **Configuration:**
  - Design: `s15850_bench`
  - Library: `NangateOpenCellLibrary_typical.lib`
  - Clock: `blif_clk_net`
  - All `???` placeholders filled in

### 3. Baseline Synthesis Results ✓
- **Clock Period:** 1000 ps
- **Slack:** 365 ps (timing met ✓)
- **Total Area:** 927.010 μm²
- **Gate Count:** 358 gates
  - Sequential: 128 (73.6%)
  - Combinational: 230 (26.4%)
- **Total Power:** 1.304 mW
  - Leakage: 17.7 μW (1.36%)
  - Internal: 1.041 mW (79.81%)
  - Switching: 245.6 μW (18.83%)

### 4. Support Scripts ✓
- **Clean Script:** `scripts/clean.sh` - Removes all generated files
- **Problem 1 Runner:** `scripts/run_problem1.sh`
- **Synthesis Runner:** `scripts/run_synthesis.sh`
- **Setup Scripts:** `_setup/new_bash_ee201a_setup`, `_setup/new_csh_ee201a_setup`

### 5. Documentation ✓
- **README.md** - Comprehensive guide with:
  - Quick start instructions
  - Problem descriptions
  - Usage examples
  - Troubleshooting tips
  - Current baseline results

## Files Copied from _bak/

- ✓ `results/wirelength.txt` - Input data for Problem 1
- ✓ `s15850.v` - RTL Verilog design
- ✓ `NangateOpenCellLibrary_typical.lib` - Technology library
- ✓ `results/results_submission.txt` - Submission template
- ✓ `_setup/new_bash_ee201a_setup` - Bash environment setup
- ✓ `_setup/new_csh_ee201a_setup` - C-shell environment setup

## TCL Script Configuration

### lab2_skeleton.tcl (Lines 30-42)
```tcl
set_db library NangateOpenCellLibrary_typical.lib
set hdl_files {s15850.v}
set DNAME s15850
set DESIGN s15850_bench
set clkpin blif_clk_net
set clk_period 1000
```

All placeholders (`???`) have been replaced with correct values.

## Quick Start Commands

### Run Problem 1 (TCL Practice)
```bash
cd /u/ee/ugrad/palatics/ee201a/lab2
bash scripts/run_problem1.sh
```

### Run Synthesis (Default)
```bash
cd /u/ee/ugrad/palatics/ee201a/lab2
bash scripts/run_synthesis.sh
```

### Run Synthesis (Custom TCL)
```bash
cd /u/ee/ugrad/palatics/ee201a/lab2
bash scripts/run_synthesis.sh tcl_scripts/custom_script.tcl
```

### Clean Up All Generated Files
```bash
cd /u/ee/ugrad/palatics/ee201a/lab2
bash scripts/clean.sh
```

## Output Files Generated

All synthesis results are saved to `results/`:

| File | Description | Size |
|------|-------------|------|
| `problem1_results.txt` | Problem 1 TCL results | 123 B |
| `synth_report_timing.txt` | Timing analysis | 2.8 KB |
| `synth_report_gates.txt` | Area breakdown | 2.4 KB |
| `synth_report_power.txt` | Power analysis | 1.2 KB |
| `s15850_synth.v` | Synthesized netlist | 38 KB |
| `s15850.sdc` | Timing constraints | 8.9 KB |

## Testing Results

### Test 1: Problem 1 TCL Script
```
✓ Script executes without errors
✓ Reads results/wirelength.txt correctly (200 entries)
✓ Calculates average: 47.39
✓ Counts wires > 50: 89
✓ Saves results to results/problem1_results.txt
```

### Test 2: Genus Synthesis (Headless)
```
✓ Sources environment setup automatically
✓ Finds genus in PATH
✓ Reads design files correctly
✓ Elaborates s15850_bench module
✓ Applies timing constraints (1000 ps clock)
✓ Completes synthesis without errors
✓ Meets timing (slack = 365 ps)
✓ Generates all output reports
✓ Creates synthesized netlist
✓ Normal exit
```

### Test 3: Clean Script
```
✓ Removes results/ directory
✓ Removes fv/ directory
✓ Removes .log files
✓ Removes .cmd files
✓ Removes genus temporary files
✓ Completes without errors
```

### Test 4: Re-run After Clean
```
✓ Problem 1 script runs successfully
✓ Synthesis script runs successfully
✓ All output files regenerated correctly
```

## Next Steps for Lab 2

### Problem 2A: Find Best Clock Period
1. Modify `set clk_period` in `tcl_scripts/lab2_skeleton.tcl`
2. Run synthesis: `bash scripts/run_synthesis.sh`
3. Check `results/synth_report_timing.txt` for slack
4. Iteratively reduce clock period until slack ≈ 0

### Problem 2B: Advanced Optimization
1. Create a copy: `cp tcl_scripts/lab2_skeleton.tcl tcl_scripts/lab2_2B.tcl`
2. Add optimization commands (high effort, multiple passes, etc.)
3. Run: `bash scripts/run_synthesis.sh tcl_scripts/lab2_2B.tcl`
4. Compare results with Problem 2A

### Problem 3A: Area vs. Delay Plot
1. Run synthesis with increasing clock periods (8+ points)
2. Extract area from `results/synth_report_gates.txt`
3. Create plot: Area (Y) vs. Clock Period (X)

### Problem 3B: Power Optimization
1. Create: `tcl_scripts/lab2_3B.tcl`
2. Add power constraints and optimization commands
3. Run synthesis with clock period = 1000 ps
4. Extract power from `results/synth_report_power.txt`

## Notes

- All scripts run in **headless mode** (no GUI required)
- Environment setup is **automatic** (sources bash setup script)
- Results are **automatically saved** to results/ directory
- Clean script **safely removes** only generated files
- All required files from `_bak/` have been **copied and configured**

## Support

See `README.md` for detailed instructions and troubleshooting.

Check `documentation/` for:
- Lab instructions
- Genus tutorial
- TCL tutorial
- Tool usage tips

---

**Lab 2 setup is complete and ready to use!**

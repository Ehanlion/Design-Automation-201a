This guide outlines the implementation of Lab 4: Physical Design using Cadence Innovus for the ISCAS89 s1494 circuit using the Nangate 45nm technology library.

Project Environment & Execution
Before beginning any implementation, ensure the environment is configured correctly. The project is hosted on the SEASnet server eeapps.seas.ucla.edu.

Project Root: lab4/

Scripts Directory: lab4/scripts/

Documentation Directory: lab4/documentation/

Setup: You must source the provided environment setup file before running any tools.


Execution Rule: Every Tcl code file must be accompanied by a shell execution script (e.g., runlab4Part1.sh) that sources the setup file and launches Innovus in batch mode (or GUI mode for debugging).
+1

Problem 1: Minimum Physical Layout Area

Objective: Determine the minimum design area (maximum cell utilization) that meets all setup/hold timing constraints and Design Rule Check (DRC) requirements.
+2

Deliverables

Tcl Script: Lastname-Firstname_UID_Username_Lab4_1.tcl.


Design Output: .invs file and .invs.dat subfolder.


Documentation: A file in lab4/documentation/problem1.md detailing the iteration steps and results.
+1

Instructions for the AI Agent

Initialize Design: Load the gate-level netlist (.v), timing constraints (.sdc), and technology LEF/Liberty files from the class directory.
+1

Iterative Floorplanning:

Modify the floorPlan command in the skeleton Tcl script.

Start with a baseline utilization (e.g., 0.7) and incrementally increase it.
+1


Constraint: Do not change the number of routing layers.

Place and Route Flow: Execute standard cells placement, pre-layout timing analysis, clock tree synthesis (CTS), and routing.


Verification: Run checkPlace, checkRoute, and timing analysis (report_timing) after routing.


Documentation Content: Outline the initial vs. final cell utilization and the worst-case setup and hold slack.

Problem 2: Timing and Power Analysis

Objective: Analyze the evolution of timing and power metrics across different physical design stages and evaluate the impact of timing-driven placement.
+2

Deliverables

Tcl Script: Lastname-Firstname_UID_Username_Lab4_2.tcl.


Documentation: A file in lab4/documentation/problem2.md containing the comparison table and an explanation of timing-driven placement features.
+2

Instructions for the AI Agent

Metric Extraction: Using the script from Problem 1, insert reporting commands (report_timing and report_power) at the following checkpoints:

Before and after placement.

After global and detailed routing.

After each optDesign command.

After RC parasitic extraction.

Toggle Timing-Driven Placement:

Create a second run where you disable timing-driven placement (typically via setPlaceMode -timingDriven false).

Repeat the metric extraction for this non-timing-driven flow.


Analysis: Compare the setup/hold slack and power consumption between the two runs.


Documentation Content: Explain what timing-driven placement does (referencing official documentation if necessary) and how it affected the results for the s1494 circuit.

Problem 3: Power Structure Modification

Objective: Add a custom power grid (stripes) to the design and assess its impact on cell utilization and timing.
+1

Deliverables

Tcl Script: Lastname-Firstname_UID_Username_Lab4_3.tcl.


Design Output: .invs file and .invs.dat subfolder.


Documentation: A file in lab4/documentation/problem3.md comparing area/timing results with and without power stripes.
+1

Instructions for the AI Agent

Add Power Stripes: Implement the addStripe command with the following specific constraints:

Layer: Metal 2 (M2).

Orientation: Vertical.

Width: 210 nm.

Spacing: 4.13.

Offset: 2 microns from the left edge.


Re-optimization: Run the full physical design flow again with these stripes in place.


Area Re-evaluation: Determine the new maximum possible cell utilization without timing or DRC violations.


Documentation Content: Explain the effect of adding power stripes on delay and power. Specifically, address whether the utilization achieved in Problem 1 is still feasible.
+1

Code Commenting Standards
Every script generated must follow this format:

Tcl
# @purpose: Initialize the floorplan with target utilization
# @logic: Using 'floorPlan' to define core area based on cell density
# why: Problem 1 requires finding the maximum utilization (min area) [cite: 46]
floorPlan -site core -util 0.8 -aspectRatio 1.0 -isRound 0
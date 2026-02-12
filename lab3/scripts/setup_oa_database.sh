#!/bin/bash

# Setup OpenAccess database from LEF, Verilog, and DEF files
# Run from any directory - script locates lab3 and runs setup from there

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment (adds OA tools to PATH)
source lab1_setup

echo ""
echo "Setting up OpenAccess database in $LAB3_DIR ..."

# Clean up any existing libraries
rm -rf DesignLib NangateLib lib.defs

# Step 1: Import LEF to create reference library
echo "Importing LEF file..."
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef
if [ $? -ne 0 ]; then
    echo "ERROR: LEF import failed!"
    exit 1
fi

# Step 2: Import Verilog netlist to create design library
echo "Importing Verilog netlist..."
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v
if [ $? -ne 0 ]; then
    echo "ERROR: Verilog import failed!"
    exit 1
fi

# Step 3: Import DEF to add physical layout
echo "Importing DEF file..."
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib
if [ $? -ne 0 ]; then
    echo "ERROR: DEF import failed!"
    exit 1
fi

echo "Setup complete."
echo ""

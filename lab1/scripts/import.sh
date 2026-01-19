#!/bin/bash

# Change to Lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Source OpenAccess environment setup
echo "Setting up OpenAccess environment..."
source lab1_setup

# Clean up any existing DesignLib and reference libraries
echo ""
echo "Cleaning up existing libraries..."
rm -rf DesignLib
rm -rf NangateLib
rm -rf lib.defs

# Step 1: Import LEF file to create reference library
echo ""
echo "Importing LEF file to create reference library..."
echo "Command: lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef"
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef

if [ $? -ne 0 ]; then
    echo "ERROR: LEF import failed!"
    exit 1
fi
echo "LEF import successful"

# Step 2: Import Verilog netlist to create design library
echo ""
echo "Importing Verilog netlist..."
echo "Command: verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v"
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v

if [ $? -ne 0 ]; then
    echo "ERROR: Verilog import failed!"
    exit 1
fi
echo "Verilog import successful"

# Step 3: Import DEF file to add physical layout information
echo ""
echo "Importing DEF file to add physical layout..."
echo "Command: def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib"
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib

if [ $? -ne 0 ]; then
    echo "ERROR: DEF import failed!"
    exit 1
fi
echo "DEF import successful"

echo ""
echo "=========================================="
echo "OpenAccess Import Complete!"
echo "=========================================="
echo "Created libraries:"
echo "  - NangateLib (reference library from LEF)"
echo "  - DesignLib (design library with s1196_bench/layout)"
echo ""
echo "You can now run: ./scripts/compile.sh"

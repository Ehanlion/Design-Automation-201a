#!/bin/bash

# Setup OpenAccess database from LEF, Verilog, and DEF files
# Run from any directory - script locates lab3 and runs setup from there

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

REQUIRED_FILES=(
    "lab1_setup"
    "NangateOpenCellLibrary.lef"
    "s1196_postrouting.v"
    "s1196_postrouting.def"
)

for required in "${REQUIRED_FILES[@]}"; do
    if [ ! -s "$required" ]; then
        echo "ERROR: Required file '$required' is missing or empty in $LAB3_DIR"
        exit 1
    fi
done

# Source OpenAccess environment (adds OA tools to PATH)
# shellcheck disable=SC1091
source lab1_setup

echo ""
echo "Setting up OpenAccess database in $LAB3_DIR ..."

# Clean up any existing libraries
rm -rf DesignLib NangateLib TechLib lib.defs cds.lib

# Step 1: Import LEF to create reference library
echo "Importing LEF file..."
lef2oa -lib NangateLib -lef NangateOpenCellLibrary.lef

# Step 2: Import Verilog netlist to create design library
echo "Importing Verilog netlist..."
verilog2oa -lib DesignLib -refLibs NangateLib -view layout -viewType maskLayout -verilog s1196_postrouting.v

# Step 3: Import DEF to add physical layout
echo "Importing DEF file..."
def2oa -lib DesignLib -cell s1196_bench -view layout -def s1196_postrouting.def -refLibs NangateLib

# Persist an input fingerprint so other scripts can detect stale libraries.
STATE_DIR="$LAB3_DIR/.oa_state"
STATE_FILE="$STATE_DIR/input.sha256"
mkdir -p "$STATE_DIR"
if command -v sha256sum >/dev/null 2>&1; then
    sha256sum NangateOpenCellLibrary.lef s1196_postrouting.v s1196_postrouting.def \
        | sha256sum | awk '{print $1}' > "$STATE_FILE"
elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 NangateOpenCellLibrary.lef s1196_postrouting.v s1196_postrouting.def \
        | shasum -a 256 | awk '{print $1}' > "$STATE_FILE"
fi

echo "Setup complete."
echo ""

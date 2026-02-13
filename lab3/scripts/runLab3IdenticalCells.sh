#!/bin/bash

# Change to lab3 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB3_DIR" || exit 1

echo "=========================================="
echo "Lab 3: Clean, Compile, and Run (Identical Cells)"
echo "=========================================="
echo ""

# Step 1: Clean up all files
echo "Step 1: Cleaning up compiled files and databases..."
rm -rf DesignLib
rm -rf NangateLib
rm -f *.o
rm -f lab3IdenticalCells
rm -f *.log
rm -f lib.defs
rm -f cds.lib
echo "Cleanup complete!"
echo ""

# Step 2: Set up OpenAccess database
echo "Step 2: Setting up OpenAccess database..."
source lab3_setup
./scripts/setupOA.sh

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Database setup failed!"
    exit 1
fi
echo "Database setup complete!"
echo ""

# Step 3: Compile the program
echo "Step 3: Compiling lab3IdenticalCells.cpp..."
echo "Command: make PROG=lab3IdenticalCells CCPATH=g++"
make PROG=lab3IdenticalCells CCPATH=g++

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Compilation failed!"
    exit 1
fi
echo "Compilation successful!"
echo ""

# Step 4: Run the program
echo "Step 4: Running lab3IdenticalCells..."
echo "=========================================="
echo ""
./lab3IdenticalCells
echo ""
echo "=========================================="
echo "Lab 3 identical-cell evaluation complete!"
echo "=========================================="

#!/bin/bash

# Change to lab3 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB3_DIR" || exit 1

echo "=========================================="
echo "Lab 3 Offskew: Clean, Compile, and Run"
echo "=========================================="
echo ""

# Step 1: Clean up all files
echo "Step 1: Cleaning up all compiled files and databases..."
rm -rf DesignLib
rm -rf NangateLib
rm -f *.o
rm -f lab3
rm -f lab3_complex
rm -f lab3_offskew
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
echo "Step 3: Compiling lab3_offskew.cpp..."
echo "Command: make lab3_offskew PROG=lab3_offskew CCPATH=g++"
make lab3_offskew PROG=lab3_offskew CCPATH=g++

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Compilation failed!"
    exit 1
fi
echo "Compilation successful!"
echo ""

# Step 4: Run the program
echo "Step 4: Running lab3_offskew..."
echo "=========================================="
echo ""
echo "OFFSKEW_TOPK=${OFFSKEW_TOPK:-6} OFFSKEW_MAX_PASSES=${OFFSKEW_MAX_PASSES:-4} OFFSKEW_TIME_BUDGET_SEC=${OFFSKEW_TIME_BUDGET_SEC:-0.10} OFFSKEW_MIN_DELTA=${OFFSKEW_MIN_DELTA:-1} OFFSKEW_MAX_RANKED_PER_PASS=${OFFSKEW_MAX_RANKED_PER_PASS:-120} OFFSKEW_KICKS_PER_PASS=${OFFSKEW_KICKS_PER_PASS:-0} OFFSKEW_MAX_KICK_DELTA=${OFFSKEW_MAX_KICK_DELTA:-2500} OFFSKEW_REFINE_ROUNDS=${OFFSKEW_REFINE_ROUNDS:-0} OFFSKEW_MAX_PAIRS_PER_ROUND=${OFFSKEW_MAX_PAIRS_PER_ROUND:-200000}"
./lab3_offskew
echo ""
echo "=========================================="
echo "Lab 3 Offskew execution complete!"
echo "=========================================="

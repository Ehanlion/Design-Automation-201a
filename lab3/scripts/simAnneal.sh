#!/bin/bash

# Compile and run the Simulated Annealing incremental placement solution.

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment
if [ ! -f "lab1_setup" ]; then
    echo "ERROR: lab1_setup not found in $LAB3_DIR"
    exit 1
fi
source lab1_setup

# Ensure OA database is set up
if [ ! -d "DesignLib" ]; then
    echo "DesignLib not found - running setup first ..."
    "$SCRIPT_DIR/setup_oa_database.sh" || exit 1
fi

# ---- Compile ----
# Check if source file exists and is non-empty
if [ ! -s "lab3_p2_sa.cpp" ]; then
    echo "ERROR: lab3_p2_sa.cpp not found or empty!"
    exit 1
fi

echo ""
echo "=== Compiling lab3_p2_sa.cpp ==="
make -s PROG=lab3_p2_sa clean 2>/dev/null || true
make PROG=lab3_p2_sa CCPATH=/usr/bin/g++

# ---- Run ----
echo ""
echo "=== Running lab3_p2_sa ==="
echo ""
./lab3_p2_sa

#!/bin/bash

# Compile and run the centroid-swapping incremental placement solution.

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment
source lab1_setup

# Ensure OA database is set up
if [ ! -d "DesignLib" ]; then
    echo "DesignLib not found - running setup first ..."
    "$SCRIPT_DIR/setup_oa_database.sh" || exit 1
fi

# Check if source file exists
if [ ! -f "lab3_p2_centroid_swap.cpp" ]; then
    echo "ERROR: lab3_p2_centroid_swap.cpp not found!"
    exit 1
fi

echo ""
echo "=== Compiling lab3_p2_centroid_swap.cpp ==="
make -s PROG=lab3_p2_centroid_swap clean 2>/dev/null || true
make PROG=lab3_p2_centroid_swap CCPATH=/usr/bin/g++

echo ""
echo "=== Running lab3_p2_centroid_swap ==="
echo ""
./lab3_p2_centroid_swap

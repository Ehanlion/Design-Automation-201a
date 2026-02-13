#!/bin/bash

# Compile and run the centroid-swapping incremental placement solution.

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment
if [ ! -f "lab1_setup" ]; then
    echo "ERROR: lab1_setup not found in $LAB3_DIR"
    exit 1
fi
# shellcheck disable=SC1091
source lab1_setup

# Always rebuild OA database before Part 2 runs to avoid cross-run contamination.
"$SCRIPT_DIR/ensure_oa_database.sh" --force

# Check if source file exists and is non-empty
if [ ! -s "lab3_p2_centroid_swap.cpp" ]; then
    echo "ERROR: lab3_p2_centroid_swap.cpp not found or empty!"
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

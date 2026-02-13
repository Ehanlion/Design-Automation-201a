#!/bin/bash

# Compile and run the identical-cell/orientation net listing helper.

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

# Always rebuild OA database before Part 2-related diagnostics to use a clean baseline.
"$SCRIPT_DIR/ensure_oa_database.sh" --force

# Check if source file exists and is non-empty
if [ ! -s "lab3_identical_cells.cpp" ]; then
    echo "ERROR: lab3_identical_cells.cpp not found or empty!"
    exit 1
fi

echo ""
echo "=== Compiling lab3_identical_cells.cpp ==="
make -s PROG=lab3_identical_cells clean 2>/dev/null || true
make PROG=lab3_identical_cells CCPATH=/usr/bin/g++

echo ""
echo "=== Running lab3_identical_cells ==="
echo ""
./lab3_identical_cells

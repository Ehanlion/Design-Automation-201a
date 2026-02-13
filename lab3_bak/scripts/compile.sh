#!/bin/bash

# Compilation script for lab3.cpp
# Run from any directory - script locates lab3, runs OA setup if needed, and invokes make

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment (needed for compilation and linking)
if [ ! -f "lab1_setup" ]; then
    echo "ERROR: lab1_setup not found in $LAB3_DIR"
    exit 1
fi
# shellcheck disable=SC1091
source lab1_setup

# Ensure OA database exists and matches current LEF/Verilog/DEF inputs.
"$SCRIPT_DIR/ensure_oa_database.sh"

# Check if source file exists and is non-empty
if [ ! -s "lab3.cpp" ]; then
    echo "ERROR: lab3.cpp not found or empty!"
    exit 1
fi

echo "Compiling lab3.cpp in $LAB3_DIR ..."
# Clean old object files to ensure fresh recompilation
make -s clean 2>/dev/null || true
# Override CCPATH since the Makefile defaults to an old GCC that may not exist
make CCPATH=/usr/bin/g++

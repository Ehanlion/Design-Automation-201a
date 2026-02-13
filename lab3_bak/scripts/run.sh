#!/bin/bash

# Run script for lab3 - executes after compilation
# Run from any directory - script locates lab3, sources OA setup, and runs lab3

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

# Source OpenAccess environment setup
if [ ! -f "lab1_setup" ]; then
    echo "ERROR: lab1_setup not found in $LAB3_DIR"
    exit 1
fi
# shellcheck disable=SC1091
source lab1_setup

# Ensure OA database exists and matches current LEF/Verilog/DEF inputs.
"$SCRIPT_DIR/ensure_oa_database.sh"

# Recompile automatically when executable is missing/outdated.
if [ ! -x "./lab3" ] || [ "lab3.cpp" -nt "lab3" ] || [ "Makefile" -nt "lab3" ]; then
    echo "lab3 executable is missing or stale - compiling first ..."
    "$SCRIPT_DIR/compile.sh"
fi

# Run the program
./lab3

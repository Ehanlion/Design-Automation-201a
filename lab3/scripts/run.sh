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
source lab1_setup

# Check if executable exists
if [ ! -x "./lab3" ]; then
    echo "ERROR: lab3 executable not found!"
    echo "Please run ./scripts/compile.sh first to compile the program."
    exit 1
fi

# Run the program
./lab3

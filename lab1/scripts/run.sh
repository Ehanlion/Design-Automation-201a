#!/bin/bash

# Change to lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Source OpenAccess environment setup
source lab1_setup

# Check if executable exists
if [ ! -f "./lab1" ]; then
    echo "ERROR: lab1 executable not found!"
    echo "Please run ./scripts/compile.sh first to compile the program."
    exit 1
fi

# Run the program
./lab1

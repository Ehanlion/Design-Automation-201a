#!/bin/bash

# Compile the program
./compilePrint.sh

# Change to Lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Source OpenAccess environment setup
source lab1_setup

# Check if executable exists
if [ ! -f "./Lab1_print" ]; then
    echo "ERROR: Lab1_print executable not found!"
    echo "Please run ./scripts/compilePrint.sh first to compile the program."
    exit 1
fi

# Run the program
./Lab1_print

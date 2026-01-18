#!/bin/bash

# Change to lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Problem 2 Compilation Script
# Compiles the lab1.cpp program for fanout analysis

echo "=========================================="
echo "Problem 2: Fanout Analysis Compilation"
echo "=========================================="
echo ""

# Check if DesignLib exists
if [ ! -d "DesignLib" ]; then
    echo "ERROR: DesignLib not found!"
    echo "Please run ./scripts/import.sh first to import the design."
    exit 1
fi

# Source OpenAccess environment setup
echo "Setting up OpenAccess environment..."
source lab1_setup

# Clean previous compilation
echo ""
echo "Cleaning previous build..."
rm -f lab1.o
rm -f lab1

# Compile the program
echo ""
echo "Compiling lab1.cpp..."
echo "Command: make lab1 CCPATH=g++"
make lab1 CCPATH=g++

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Compilation failed!"
    echo ""
    echo "Common issues:"
    echo "  - Make sure Problems 2 and 3 code is complete (or comment out incomplete parts)"
    echo "  - Check for undefined variables (fanout, etc.)"
    echo "  - Verify OpenAccess environment is properly set up"
    exit 1
fi

echo ""
echo "=========================================="
echo "Compilation Successful!"
echo "=========================================="
echo ""
echo "To run the program:"
echo "  ./lab1"
echo ""
echo "The program will:"
echo "  1. Open the design from DesignLib"
echo "  2. Print design names and nets"
echo "  3. Compute and display average fanout (Problem 2)"
echo "  4. Compute and display average wirelength (Problem 3)"

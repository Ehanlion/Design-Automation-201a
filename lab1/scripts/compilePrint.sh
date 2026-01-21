#!/bin/bash

# Change to Lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Compilation Script for Lab1_print.cpp
# Compiles the Lab1_print.cpp program with histogram and terminal plotting

echo "=========================================="
echo "Lab1_print: Compilation with Printing"
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
rm -f Lab1_print.o
rm -f Lab1_print

# Compile the program
echo ""
echo "Compiling Lab1_print.cpp..."
echo "Command: make Lab1_print PROG=Lab1_print CCPATH=g++"
make Lab1_print PROG=Lab1_print CCPATH=g++


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

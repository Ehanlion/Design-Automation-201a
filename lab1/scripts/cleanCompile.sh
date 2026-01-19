#!/bin/bash

# Change to lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Problem 2 Cleanup Script
# Removes compiled binaries and object files

echo "Cleaning up Problem 2 compilation files..."

# Remove main file and test files
rm -f lab1.o
rm -f lab1
rm -f lab1_test.o
rm -f lab1_test

echo "Cleanup complete!"
echo ""
echo "Note: This does not remove DesignLib or NangateLib."
echo "To clean those, run: ./scripts/cleanImport.sh"

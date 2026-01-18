#!/bin/bash

# Problem 2 Cleanup Script
# Removes compiled binaries and object files

echo "Cleaning up Problem 2 compilation files..."

rm -f lab1.o
rm -f lab1

echo "Cleanup complete!"
echo ""
echo "Note: This does not remove DesignLib or NangateLib."
echo "To clean those, run: ./scripts/problem1-clean.sh"

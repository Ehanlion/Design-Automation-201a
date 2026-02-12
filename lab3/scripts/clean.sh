#!/bin/bash

# Clean script for lab3 - removes compiled files and design library
# Run from any directory - script locates lab3 and cleans from there

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

echo "Cleaning lab3 build artifacts in $LAB3_DIR ..."

# Remove all compiled object files
rm -f *.o

# Remove executables only (not .cpp source files)
rm -f lab3 lab3_p2_greedy_lock lab3_p2_sa lab3_p2_centroid_swap

# Remove design library and related OpenAccess databases
rm -rf DesignLib
rm -rf NangateLib
rm -rf TechLib

# Remove OpenAccess config files
rm -f lib.defs cds.lib

# Remove log files
rm -f *.log

echo "Clean complete."

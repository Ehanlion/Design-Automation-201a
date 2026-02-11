#!/bin/bash

# Clean script for lab3 - removes compiled files and generated databases
# Run from any directory - script locates lab3 and cleans from there

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

echo "Cleaning lab3 build artifacts in $LAB3_DIR ..."

# Remove OpenAccess database directories (from setup_oa_database.sh)
rm -rf DesignLib
rm -rf NangateLib
rm -rf TechLib

# Remove compiled objects and executable (from Makefile)
rm -f lab3.o lab3
rm -f *.o

# Remove log files
rm -f *.log

# Remove OpenAccess config files
rm -f lib.defs
rm -f cds.lib

echo "Clean complete."

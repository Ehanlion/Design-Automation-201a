#!/bin/bash

# Change to lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Problem 1 Cleanup Script
# Removes OpenAccess libraries and log files created during import

echo "Cleaning up Problem 1 files..."

rm -rf DesignLib
rm -rf TechLib
rm -rf NangateLib
rm -f lib.defs
rm -f *.log

echo "Cleanup complete!"

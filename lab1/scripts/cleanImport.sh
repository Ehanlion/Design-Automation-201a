#!/bin/bash

# Problem 1 Cleanup Script
# Removes OpenAccess libraries and log files created during import

echo "Cleaning up Problem 1 files..."

rm -rf DesignLib
rm -rf TechLib
rm -rf NangateLib
rm -f lib.defs
rm -f *.log

echo "Cleanup complete!"

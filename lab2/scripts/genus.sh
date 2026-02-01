#!/bin/bash

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# Get the lab2 root directory (parent of scripts)
LAB2_DIR="$( cd "$SCRIPT_DIR/.." && pwd )"

# Change to lab2 directory
cd "$LAB2_DIR"

# Source the setup file
SETUP_SCRIPT="new_bash_ee201a_setup"
if [ -f "$SETUP_SCRIPT" ]; then
    echo "Sourcing environment setup..."
    source "$SETUP_SCRIPT"
else
    echo "Warning: Setup script '$SETUP_SCRIPT' not found!"
    echo "         Genus may not be available in PATH"
fi

# Launch Genus interactively
genus

# Cleanup
echo "Cleaning up Genus-generated files..."
rm -f genus.cmd* genus_* .genus* .cadence genus.log*
echo "Done."
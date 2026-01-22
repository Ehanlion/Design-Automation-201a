#!/bin/bash

# Change to Lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Verify DesignLib exists (required for executables to run)
if [ ! -d "./DesignLib" ]; then
    echo "WARNING: DesignLib directory not found!"
    echo "Executables may fail. Run ./scripts/import.sh first to create DesignLib."
    echo ""
fi

# Find and run executables in submissions directory
SUBMISSIONS_DIR="$LAB1_DIR/submissions"

if [ ! -d "$SUBMISSIONS_DIR" ]; then
    echo "ERROR: Submissions directory not found!"
    exit 1
fi

# Find all executables in submission subdirectories
find "$SUBMISSIONS_DIR" -type f -executable | while read -r executable; do
    echo "Running: $executable"
    echo "----------------------------------------"
    "$executable"
    echo "----------------------------------------"
    echo ""
done

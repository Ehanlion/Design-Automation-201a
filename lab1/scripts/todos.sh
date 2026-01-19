#!/bin/bash

# Change to Lab1 directory (parent of scripts/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

# Find all TODO comments in Lab1 directory
# Searches recursively through all files

echo "=========================================="
echo "Finding all TODO/todo comments in Lab1/"
echo "=========================================="
echo ""

# Use grep to find TODO/todo (case-insensitive)
# -r: recursive
# -n: show line numbers
# -i: case-insensitive
# Exclude binary files and large generated directories

grep -r -n -i "todo" . \
    --exclude-dir="DesignLib" \
    --exclude-dir="NangateLib" \
    --exclude-dir=".git" \
    --exclude="*.o" \
    --exclude="*.log" \
    --exclude="*.oa" \
    --exclude="*.tag" \
    --exclude="todos.sh" \
    2>/dev/null

echo ""
echo "=========================================="
echo "Search complete!"
echo "=========================================="

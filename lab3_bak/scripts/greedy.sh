#!/bin/bash

# Compile + run script for the first Lab 3 Problem 2 iteration:
# lab3_p2_greedy_lock.cpp

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

if [ ! -f "lab1_setup" ]; then
    echo "ERROR: lab1_setup not found in $LAB3_DIR"
    exit 1
fi
# shellcheck disable=SC1091
source lab1_setup

# Always rebuild OA database before Part 2 runs to avoid cross-run contamination.
"$SCRIPT_DIR/ensure_oa_database.sh" --force

if [ ! -s "lab3_p2_greedy_lock.cpp" ]; then
    echo "ERROR: lab3_p2_greedy_lock.cpp not found or empty!"
    exit 1
fi

echo "Compiling lab3_p2_greedy_lock.cpp in $LAB3_DIR ..."
make -s PROG=lab3_p2_greedy_lock clean 2>/dev/null || true
make PROG=lab3_p2_greedy_lock CCPATH=/usr/bin/g++

echo
echo "Running lab3_p2_greedy_lock ..."
./lab3_p2_greedy_lock

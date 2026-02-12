#!/bin/bash

# Compile + run script for the first Lab 3 Problem 2 iteration:
# lab3_p2_greedy_lock.cpp

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

source lab1_setup

echo "Refreshing OA database from LEF/Verilog/DEF ..."
"$SCRIPT_DIR/setup_oa_database.sh"

echo "Compiling lab3_p2_greedy_lock.cpp in $LAB3_DIR ..."
make -s PROG=lab3_p2_greedy_lock clean 2>/dev/null || true
make PROG=lab3_p2_greedy_lock CCPATH=/usr/bin/g++

echo
echo "Running lab3_p2_greedy_lock ..."
./lab3_p2_greedy_lock

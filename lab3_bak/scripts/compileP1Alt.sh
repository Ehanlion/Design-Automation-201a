#!/bin/bash

# Compile script for lab3_p1_alt.cpp (Part 1 alternate approach).

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

if [ ! -f "lab1_setup" ]; then
	echo "ERROR: lab1_setup not found in $LAB3_DIR"
	exit 1
fi
source lab1_setup

if [ ! -d "DesignLib" ]; then
	echo "DesignLib not found - running setup first ..."
	"$SCRIPT_DIR/setup_oa_database.sh" || exit 1
fi

if [ ! -s "lab3_p1_alt.cpp" ]; then
	echo "ERROR: lab3_p1_alt.cpp not found or empty!"
	exit 1
fi

echo "Compiling lab3_p1_alt.cpp in $LAB3_DIR ..."
make -s clean PROG=lab3_p1_alt 2>/dev/null || true
make PROG=lab3_p1_alt CCPATH=/usr/bin/g++

#!/bin/bash

# Run script for lab3_p1_alt executable.

./compileP1Alt.sh

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

if [ ! -f "lab1_setup" ]; then
	echo "ERROR: lab1_setup not found in $LAB3_DIR"
	exit 1
fi
source lab1_setup

if [ ! -x "./lab3_p1_alt" ]; then
	echo "ERROR: lab3_p1_alt executable not found!"
	echo "Please run ./scripts/compile_p1_alt.sh first."
	exit 1
fi

./lab3_p1_alt

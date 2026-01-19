#!/bin/bash

# Compile the program
./compileTest.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

source lab1_setup

if [ ! -f "./Lab1_test" ]; then
    echo "ERROR: Lab1_test executable not found!"
    echo "Please run ./scripts/compileTest.sh first to compile the program."
    exit 1
fi

./Lab1_test

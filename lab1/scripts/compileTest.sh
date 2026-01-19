#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB1_DIR" || exit 1

source lab1_setup

rm -f lab1_test.o
rm -f lab1_test

g++ -Wno-ctor-dtor-privacy -O -I/w/class.1/ee/ee201o/ee201ota/oa/include/oa \
    -I/w/class.1/ee/ee201o/ee201ota/oa/include \
    -c lab1_test.cpp -o lab1_test.o

if [ $? -ne 0 ]; then
    echo "ERROR: Compilation failed!"
    exit 1
fi

g++ -Wno-ctor-dtor-privacy -O -o lab1_test lab1_test.o \
    -L/w/class.1/ee/ee201o/ee201ota/oa/lib/linux_rhel40_64/opt \
    -loaCommon -loaBase -loaPlugIn -loaDM -loaTech -loaDesign -ldl

if [ $? -ne 0 ]; then
    echo "ERROR: Linking failed!"
    exit 1
fi

echo "Compilation successful!"

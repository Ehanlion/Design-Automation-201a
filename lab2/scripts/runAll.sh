#!/bin/bash

# Run all run scripts for parts 1, 2A, 3A, 3B, and 2B

# Start by cleaning the directory
bash clean.sh

# Run the benchmakr
bash runBenchmark.sh

# Run problem 1
bash run1.sh

# Run problem 2
bash run2A.sh
bash run2B.sh

# Run problem 3
bash run3A.sh
bash run3B.sh

# update the results submission file
bash updateResults.sh

echo "========================================"
echo "All scripts completed successfully!"
echo "========================================"
echo ""
#!/bin/bash

# 2. Source the setup file
echo "Sourcing environment setup..."
source new_base_ee201a_setup
genus

# 4. Cleanup
echo "Cleaning up log files..."
rm -f genus.cmd* genus.log*
echo "Done."
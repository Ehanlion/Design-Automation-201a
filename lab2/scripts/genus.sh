#!/bin/bash

# 2. Source the setup file
echo "Sourcing environment setup..."
source new_base_ee201a_setup
genus

# 4. Cleanup
echo "Cleaning up Genus-generated files..."
rm -f genus.cmd* genus_* .genus* .cadence genus.log*
echo "Done."
#!/usr/bin/env bash
set -euo pipefail

# Cleans all Innovus-generated artifacts from the lab4 directory.
# Safe to re-run; only removes generated files, never source files.
#
# Usage:
#   ./scripts/clean.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "$LAB4_DIR"

echo "Cleaning Lab 4 generated files..."
echo "Working directory: $LAB4_DIR"

# Innovus runtime artifacts in lab4 root
rm -f ./*.log ./*.log.* ./*.cmd ./*.cmd.*
rm -f ./innovus.log* ./innovus.cmd* ./streamOut.map
rm -rf ./timingReports ./INNOVUS*
rm -rf ./.cadence ./.stylus*
rm -rf ./innovus_temp_* /tmp/innovus_temp_*_palatics_* 2>/dev/null || true

# Remove the .nfs files
rm -f ./.nfs*

# All generated outputs (Part 1/2/3 static output and sweep runs)
# NFS stale handles from killed processes may linger; ignore those errors.
rm -rf ./output/* 2>/dev/null || true
rm -f ./output/part2_latest_run.txt 2>/dev/null || true
rm -f ./*part2*.log ./*part2*.log.* 2>/dev/null || true

echo "Cleanup complete."

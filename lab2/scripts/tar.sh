#!/bin/bash

# Script to prepare Lab2 submission directory
# Creates submission directory and copies all required files with proper naming

# Set the prefix for file names
PREFIX="Owen-Ethan_905452983_palatics"

# Base directory (where this script is run from)
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Submissions directory
SUBMISSIONS_DIR="${BASE_DIR}/submissions"

# Submission directory name
SUBMISSION_DIR_NAME="${PREFIX}_Lab2"

# Full path to submission directory
SUBMISSION_DIR="${SUBMISSIONS_DIR}/${SUBMISSION_DIR_NAME}"

# Create submissions directory if it doesn't exist
mkdir -p "${SUBMISSIONS_DIR}"

# Create submission directory
echo "Creating submission directory: ${SUBMISSION_DIR}"
mkdir -p "${SUBMISSION_DIR}"

# Copy files with proper naming
echo "Copying files to submission directory..."

# 1. results_submission.txt
cp "${BASE_DIR}/results_submission.txt" \
   "${SUBMISSION_DIR}/${PREFIX}_results_submission.txt"

# 2. lab2_2B.tcl
cp "${BASE_DIR}/lab2_2B.tcl" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2_2B.tcl"

# 3. 2B synthesized Verilog
cp "${BASE_DIR}/results/2B_s15850_synth.v" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2_2B.v"

# 4. lab2_3B.tcl
cp "${BASE_DIR}/lab2_3B.tcl" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2_3B.tcl"

# 5. 3B synthesized Verilog
cp "${BASE_DIR}/results/3B_s15850_synth.v" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2_3B.v"

# 6. lab2_1.tcl
cp "${BASE_DIR}/lab2_1.tcl" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2_1.tcl"

# 7. Lab report PDF
cp "${BASE_DIR}/documentation/${PREFIX}_Lab2.pdf" \
   "${SUBMISSION_DIR}/${PREFIX}_Lab2.pdf"

# Set permissions: read and execute for group and others
echo "Setting permissions..."
chmod -R go+rx "${SUBMISSION_DIR}"

# Create tar.gz archive
PIN="1234"
ARCHIVE_NAME="${PREFIX}_Lab2_pin${PIN}.tar.gz"
ARCHIVE_PATH="${SUBMISSIONS_DIR}/${ARCHIVE_NAME}"

echo ""
echo "Creating tar.gz archive: ${ARCHIVE_PATH}"
cd "${SUBMISSIONS_DIR}"
tar -czf "${ARCHIVE_NAME}" "${SUBMISSION_DIR_NAME}"

# Set permissions on archive
chmod go+rx "${ARCHIVE_PATH}"

# Verify files
echo ""
echo "Submission directory created: ${SUBMISSION_DIR}"
echo ""
echo "Files in submission directory:"
ls -lh "${SUBMISSION_DIR}"
echo ""
echo "Permissions:"
ls -ld "${SUBMISSION_DIR}"
ls -l "${SUBMISSION_DIR}"

echo ""
echo "Archive created: ${ARCHIVE_PATH}"
ls -lh "${ARCHIVE_PATH}"

echo ""
echo "Submission preparation complete!"

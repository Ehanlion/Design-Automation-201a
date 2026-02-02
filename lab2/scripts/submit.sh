#!/bin/bash

# Script to submit Lab2 tarball to the submission directory
# Copies the tar.gz file and verifies it was successfully copied

# Set the prefix for file names
PREFIX="Owen-Ethan_905452983_palatics"

# Base directory (where this script is run from)
BASE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Submissions directory
SUBMISSIONS_DIR="${BASE_DIR}/submissions"

# Archive name
PIN="1234"
ARCHIVE_NAME="${PREFIX}_Lab2_pin${PIN}.tar.gz"
ARCHIVE_PATH="${SUBMISSIONS_DIR}/${ARCHIVE_NAME}"

# Submission location
SUBMIT_DIR="/w/class.1/ee/ee201o/ee201ot2/submission/lab2"
SUBMIT_PATH="${SUBMIT_DIR}/${ARCHIVE_NAME}"

# Check if archive exists
if [ ! -f "${ARCHIVE_PATH}" ]; then
    echo "Error: Archive not found at ${ARCHIVE_PATH}"
    echo "Please run prepare_submission.sh first to create the archive."
    exit 1
fi

# Copy the archive to submission directory
echo "Copying archive to submission location..."
echo "Source: ${ARCHIVE_PATH}"
echo "Destination: ${SUBMIT_PATH}"

cp "${ARCHIVE_PATH}" "${SUBMIT_PATH}"

if [ $? -ne 0 ]; then
    echo "Error: Failed to copy archive to submission directory"
    exit 1
fi

echo ""
echo "Archive copied successfully!"
echo ""

# Verify the file exists and can be read
echo "Verifying submission..."
if [ -f "${SUBMIT_PATH}" ]; then
    echo "✓ File exists at submission location"
    
    # Try to get file size and permissions
    if ls -lh "${SUBMIT_PATH}" > /dev/null 2>&1; then
        echo "✓ File is readable"
        echo ""
        echo "File details:"
        ls -lh "${SUBMIT_PATH}"
    else
        echo "Warning: File exists but cannot read details (this may be normal)"
    fi
    
    # Try to verify it's a valid tar.gz file by checking the first few bytes
    if file "${SUBMIT_PATH}" > /dev/null 2>&1; then
        echo ""
        echo "File type verification:"
        file "${SUBMIT_PATH}"
    fi
    
    # Try to list contents (just verify it's a valid tar.gz)
    echo ""
    echo "Verifying archive integrity..."
    if tar -tzf "${SUBMIT_PATH}" > /dev/null 2>&1; then
        echo "✓ Archive is valid and readable"
        echo ""
        echo "Archive contents:"
        tar -tzf "${SUBMIT_PATH}"
    else
        echo "Warning: Could not verify archive contents (this may be normal due to permissions)"
    fi
    
    echo ""
    echo "Submission verification complete!"
    echo "Archive submitted to: ${SUBMIT_PATH}"
else
    echo "Error: File not found at submission location after copy"
    exit 1
fi

#!/bin/bash

# Submit the Lab3 tarball to the submission directory
# Make sure to run tar.sh first to create the tarball

# Configuration (must match tar.sh)
PREFIX="Owen-Ethan_905452983_palatics"
DIR_NAME="${PREFIX}_Lab3"
PIN="1234"  # TODO: Replace with your actual 4-digit PIN before submitting

# Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"  # Lab3 directory (parent of scripts/)
SUBMISSIONS_DIR="$LAB3_DIR/submission"

# Construct tar file name
TAR_FILE="${DIR_NAME}_pin${PIN}.tar.gz"
TAR_PATH="${SUBMISSIONS_DIR}/${TAR_FILE}"

# Submission location
SUBMIT_DIR="/w/class.1/ee/ee201o/ee201ot2/submission/lab3"
SUBMIT_PATH="${SUBMIT_DIR}/${TAR_FILE}"

# Check if tar file exists
if [ ! -f "$TAR_PATH" ]; then
    echo "ERROR: Tar file not found: $TAR_PATH"
    echo "Please run tar.sh first to create the tarball."
    echo "  Usage: bash scripts/tar.sh"
    exit 1
fi

echo "========================================="
echo "Submitting Lab3"
echo "========================================="
echo "Source:      $TAR_PATH"
echo "Destination: $SUBMIT_PATH"
echo ""

# Ensure tar file has correct permissions before copying
chmod go+rx "$TAR_PATH"

# Copy to submission directory
cp "$TAR_PATH" "$SUBMIT_PATH"

if [ $? -ne 0 ]; then
    echo "ERROR: Failed to copy archive to submission directory."
    echo "Check that the submission directory exists and is writable."
    exit 1
fi

# Set permissions on copied file
chmod go+rx "$SUBMIT_PATH"

echo "Archive copied successfully!"
echo ""

# Verify the file exists and can be read
echo "Verifying submission..."
if [ -f "$SUBMIT_PATH" ]; then
    echo "  File exists at submission location"

    # Try to get file details
    if ls -lh "$SUBMIT_PATH" > /dev/null 2>&1; then
        echo "  File is readable"
        echo ""
        echo "File details:"
        ls -lh "$SUBMIT_PATH"
    else
        echo "  Warning: File exists but cannot read details (this may be normal due to directory permissions)"
    fi

    # Verify it's a valid tar.gz
    echo ""
    echo "Verifying archive integrity..."
    if tar -tzf "$SUBMIT_PATH" > /dev/null 2>&1; then
        echo "  Archive is valid and readable"
        echo ""
        echo "Archive contents:"
        tar -tzf "$SUBMIT_PATH"
    else
        echo "  Warning: Could not verify archive contents (this may be normal due to permissions)"
    fi

    echo ""
    echo "========================================="
    echo "Submission complete!"
    echo "========================================="
    echo "Submitted: $SUBMIT_PATH"
else
    echo "ERROR: File not found at submission location after copy."
    echo "Note: You may not be able to list the directory, but the file may still exist."
    echo "The submission directory has restricted read permissions."
fi

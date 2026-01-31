#!/bin/bash

# Submit the Lab1 tarball to the submission directory
# Make sure to run tar.sh first to create the tarball

# Configuration (must match tar.sh)
DIR_NAME="Owen-Ethan_905452983_palatics_Lab1"
PIN="pin1234"  # TODO: Replace with your actual PIN before submitting

# Change to scripts directory to ensure relative paths work
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"  # Lab1 directory (parent of scripts/)
SUBMISSIONS_DIR="$LAB1_DIR/submissions"  # Submissions directory in Lab1

# Change to submissions directory
cd "$SUBMISSIONS_DIR" || exit 1

# Construct tar file name
TAR_FILE="${DIR_NAME}_${PIN}.tar.gz"

# Check if tar file exists
if [ ! -f "$TAR_FILE" ]; then
    echo "ERROR: Tar file not found: $TAR_FILE"
    echo "Please run ./scripts/tar.sh first to create the tarball."
    exit 1
fi

# Submit path
SUBMIT_PATH="/w/class.1/ee/ee201o/ee201ot2/submission/lab1/"

echo "Submitting to: $SUBMIT_PATH"
echo "Tar file: $TAR_FILE"

# Create submission directory if it doesn't exist
if [ ! -d "$SUBMIT_PATH" ]; then
    echo "Creating submission directory: $SUBMIT_PATH"
    mkdir -p "$SUBMIT_PATH"
fi

# Ensure tar file has correct permissions before copying
chmod go+rx "$TAR_FILE"

# Copy to submission directory
cp "$TAR_FILE" "$SUBMIT_PATH"

# Set permissions on copied file in submission directory
chmod go+rx "$SUBMIT_PATH/$TAR_FILE"

echo ""
echo "Verifying submission (file details):"
if command -v ll >/dev/null 2>&1; then
    ll "$SUBMIT_PATH$TAR_FILE"
else
    ls -l "$SUBMIT_PATH$TAR_FILE"
fi
echo ""

echo "Submission complete!"
echo "Submitted file: $SUBMIT_PATH$TAR_FILE"

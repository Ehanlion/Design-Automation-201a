#!/usr/bin/env bash
set -euo pipefail

# Submit the Lab4 tarball to the class submission directory.
# Make sure to run tar.sh first.

# Configuration (must match tar.sh)
PREFIX="Owen-Ethan_905452983_palatics"
DIR_NAME="${PREFIX}_Lab4"
PIN="1234"  # TODO: Replace with your actual 4-digit PIN before submitting

# Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB4_DIR="$(dirname "$SCRIPT_DIR")"
SUBMISSIONS_DIR="$LAB4_DIR/submission"

TAR_FILE="${DIR_NAME}_pin${PIN}.tar.gz"
TAR_PATH="${SUBMISSIONS_DIR}/${TAR_FILE}"

# Submission location
SUBMIT_DIR="/w/class.1/ee/ee201o/ee201ot2/submission/lab4"
SUBMIT_PATH="${SUBMIT_DIR}/${TAR_FILE}"

if [ ! -f "$TAR_PATH" ]; then
    echo "ERROR: Tar file not found: $TAR_PATH"
    echo "Run tar.sh first:"
    echo "  bash scripts/tar.sh"
    exit 1
fi

echo "========================================="
echo "Submitting Lab4"
echo "========================================="
echo "Source:      $TAR_PATH"
echo "Destination: $SUBMIT_PATH"
echo ""

# Ensure tarball is readable by graders
chmod go+rx "$TAR_PATH"

cp "$TAR_PATH" "$SUBMIT_PATH"
chmod go+rx "$SUBMIT_PATH" 2>/dev/null || true

echo "Archive copied successfully."
echo ""
echo "Attempting post-copy verification..."
if [ -f "$SUBMIT_PATH" ]; then
    echo "  File exists at submission path."
    ls -lh "$SUBMIT_PATH" 2>/dev/null || \
        echo "  Note: could not list file details (directory listing may be restricted)."
else
    echo "  Note: cannot confirm with -f (this can happen with restricted permissions)."
fi

echo ""
echo "========================================="
echo "Submission complete"
echo "========================================="
echo "Submitted: $SUBMIT_PATH"

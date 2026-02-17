#!/bin/bash

# TAR the Lab3 files (use submit.sh to submit)
# Create files:
# directory: Owen-Ethan_905452983_palatics_Lab3/
# files in directory:
# - Owen-Ethan_905452983_palatics_Lab3.cpp               <- lab3.cpp
# - Owen-Ethan_905452983_palatics_Lab3                    <- lab3 (compiled binary)
# - Owen-Ethan_905452983_palatics_Lab3.pdf                <- documentation PDF
# - Owen-Ethan_905452983_palatics_Lab3_results_submission.txt <- results_submission.txt

# Then compress the directory into a tar file
# Tar file: Owen-Ethan_905452983_palatics_Lab3_pin1234.tar.gz

# Submit file using submit.sh script
# but make sure to set perms with chmod -R go+rx for all files
# submit path: /w/class.1/ee/ee201o/ee201ot2/submission/lab3/

# Configuration
PREFIX="Owen-Ethan_905452983_palatics"
DIR_NAME="${PREFIX}_Lab3"
PIN="1234"  # TODO: Replace with your actual 4-digit PIN before submitting

# Resolve directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"  # Lab3 directory (parent of scripts/)
SUBMISSIONS_DIR="$LAB3_DIR/submission"

# Create submissions directory if it doesn't exist
if [ ! -d "$SUBMISSIONS_DIR" ]; then
    echo "Creating submissions directory: $SUBMISSIONS_DIR/"
    mkdir -p "$SUBMISSIONS_DIR"
fi

# Change to submissions directory
cd "$SUBMISSIONS_DIR" || exit 1

# Clean up any existing directory
if [ -d "$DIR_NAME" ]; then
    echo "Removing existing $DIR_NAME directory..."
    rm -rf "$DIR_NAME"
fi

# Create the submission directory
echo "Creating directory: $DIR_NAME/"
mkdir -p "$DIR_NAME"

# ---- 1. Copy lab3.cpp -> Owen-Ethan_905452983_palatics_Lab3.cpp ----
if [ -f "$LAB3_DIR/lab3.cpp" ]; then
    echo "Copying lab3.cpp -> $DIR_NAME/${PREFIX}_Lab3.cpp"
    cp "$LAB3_DIR/lab3.cpp" "$DIR_NAME/${PREFIX}_Lab3.cpp"
else
    echo "ERROR: lab3.cpp not found in $LAB3_DIR!"
    exit 1
fi

# ---- 2. Copy lab3 binary -> Owen-Ethan_905452983_palatics_Lab3 ----
if [ -f "$LAB3_DIR/lab3" ]; then
    echo "Copying lab3 -> $DIR_NAME/${PREFIX}_Lab3"
    cp "$LAB3_DIR/lab3" "$DIR_NAME/${PREFIX}_Lab3"
else
    echo "WARNING: lab3 executable not found! Make sure to compile it first (make)."
fi

# ---- 3. Copy PDF report ----
PDF_FOUND=false
if [ -f "$LAB3_DIR/documentation/${PREFIX}_Lab3.pdf" ]; then
    echo "Copying PDF from documentation directory..."
    cp "$LAB3_DIR/documentation/${PREFIX}_Lab3.pdf" "$DIR_NAME/${PREFIX}_Lab3.pdf"
    PDF_FOUND=true
elif [ -f "$SUBMISSIONS_DIR/${PREFIX}_Lab3.pdf" ]; then
    echo "Copying PDF from submission directory..."
    cp "${PREFIX}_Lab3.pdf" "$DIR_NAME/${PREFIX}_Lab3.pdf"
    PDF_FOUND=true
elif [ -f "$LAB3_DIR/${PREFIX}_Lab3.pdf" ]; then
    echo "Copying PDF from Lab3 directory..."
    cp "$LAB3_DIR/${PREFIX}_Lab3.pdf" "$DIR_NAME/${PREFIX}_Lab3.pdf"
    PDF_FOUND=true
fi

if [ "$PDF_FOUND" = false ]; then
    echo "WARNING: PDF file not found! You need ${PREFIX}_Lab3.pdf"
    echo "  Place it in one of these locations:"
    echo "    - $LAB3_DIR/documentation/"
    echo "    - $SUBMISSIONS_DIR/"
    echo "    - $LAB3_DIR/"
fi

# ---- 4. Copy results_submission.txt ----
if [ -f "$LAB3_DIR/results_submission.txt" ]; then
    echo "Copying results_submission.txt -> $DIR_NAME/${PREFIX}_Lab3_results_submission.txt"
    cp "$LAB3_DIR/results_submission.txt" "$DIR_NAME/${PREFIX}_Lab3_results_submission.txt"
else
    echo "WARNING: results_submission.txt not found in $LAB3_DIR!"
fi

# Set permissions on all files (required for grading)
echo ""
echo "Setting permissions (chmod -R go+rx)..."
chmod -R go+rx "$DIR_NAME"

# Create tar.gz file in submission directory
TAR_FILE="${DIR_NAME}_pin${PIN}.tar.gz"
echo "Creating tar file: $TAR_FILE"
tar -czf "$TAR_FILE" "$DIR_NAME"

if [ -f "$TAR_FILE" ]; then
    # Set permissions on tar file (required for submission)
    chmod go+rx "$TAR_FILE"

    echo ""
    echo "========================================="
    echo "Tarball created successfully!"
    echo "========================================="
    echo "File: $TAR_FILE"
    echo "Size: $(du -h "$TAR_FILE" | cut -f1)"
    echo "Location: $(pwd)/$TAR_FILE"
    echo ""
    echo "Contents:"
    tar -tzf "$TAR_FILE"
    echo ""
    echo "To submit, run: bash scripts/submit.sh"
else
    echo "ERROR: Failed to create tar file!"
    exit 1
fi

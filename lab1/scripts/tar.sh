#!/bin/bash

# TAR the Lab1 files (use submit.sh to submit)
# Create files:
# directory: Owen-Ethan_905452983_palatics_Lab1/
# files in directory: 
# - Owen-Ethan_905452983_palatics_Lab1.cpp <- Lab1.cpp
# - Owen-Ethan_905452983_palatics_Lab1 <- Lab1
# - Owen-Ethan_905452983_palatics_Lab1.pdf <- created project pdf

# Then compress the directory into a tar file
# Tar file: Owen-Ethan_905452983_palatics_Lab1_pin1234.tar.gz

# Submit file using submit.sh script
# but make sure to set perms with chmod -R go+rx for all files
# submit path: /w/class.1/ee/ee201o/ee201ot2/submission/lab1/

# Configuration
DIR_NAME="Owen-Ethan_905452983_palatics_Lab1"
PIN="pin1234"  # TODO: Replace with your actual PIN before submitting

# Change to scripts directory to ensure relative paths work
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
LAB1_DIR="$(dirname "$SCRIPT_DIR")"  # Lab1 directory (parent of scripts/)
SUBMISSIONS_DIR="$LAB1_DIR/submissions"  # Submissions directory in Lab1

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

# Copy Lab1.cpp from Lab1 directory to Owen-Ethan_905452983_palatics_Lab1.cpp
if [ -f "$LAB1_DIR/Lab1.cpp" ]; then
    echo "Copying Lab1.cpp -> $DIR_NAME/Owen-Ethan_905452983_palatics_Lab1.cpp"
    cp "$LAB1_DIR/Lab1.cpp" "$DIR_NAME/Owen-Ethan_905452983_palatics_Lab1.cpp"
else
    echo "ERROR: Lab1.cpp not found in $LAB1_DIR!"
    exit 1
fi

# Copy Lab1 executable from Lab1 directory to Owen-Ethan_905452983_palatics_Lab1
if [ -f "$LAB1_DIR/Lab1" ]; then
    echo "Copying Lab1 -> $DIR_NAME/Owen-Ethan_905452983_palatics_Lab1"
    cp "$LAB1_DIR/Lab1" "$DIR_NAME/Owen-Ethan_905452983_palatics_Lab1"
else
    echo "WARNING: Lab1 executable not found! Make sure to compile it first."
fi

# Copy PDF - check multiple locations
PDF_FOUND=false
if [ -f "Owen-Ethan_905452983_palatics_Lab1.pdf" ]; then
    echo "Copying PDF file from submissions directory..."
    cp "Owen-Ethan_905452983_palatics_Lab1.pdf" "$DIR_NAME/"
    PDF_FOUND=true
elif [ -f "$LAB1_DIR/documentation/Owen-Ethan_905452983_palatics_Lab1.pdf" ]; then
    echo "Copying PDF file from documentation directory..."
    cp "$LAB1_DIR/documentation/Owen-Ethan_905452983_palatics_Lab1.pdf" "$DIR_NAME/"
    PDF_FOUND=true
elif [ -f "$LAB1_DIR/Owen-Ethan_905452983_palatics_Lab1.pdf" ]; then
    echo "Copying PDF file from Lab1 directory..."
    cp "$LAB1_DIR/Owen-Ethan_905452983_palatics_Lab1.pdf" "$DIR_NAME/"
    PDF_FOUND=true
fi

if [ "$PDF_FOUND" = false ]; then
    echo "WARNING: PDF file not found! You need to create Owen-Ethan_905452983_palatics_Lab1.pdf"
    echo "  Place it in one of these locations:"
    echo "    - $SUBMISSIONS_DIR/"
    echo "    - $LAB1_DIR/documentation/"
    echo "    - $LAB1_DIR/"
fi

# Set permissions on all files
echo "Setting permissions (chmod -R go+rx)..."
chmod -R go+rx "$DIR_NAME"

# Create tar.gz file in submissions directory
TAR_FILE="${DIR_NAME}_${PIN}.tar.gz"
echo "Creating tar file: $TAR_FILE"
tar -czf "$TAR_FILE" "$DIR_NAME"

if [ -f "$TAR_FILE" ]; then
    echo "Successfully created: $TAR_FILE"
    echo "File size: $(du -h "$TAR_FILE" | cut -f1)"
    echo "Location: $(pwd)/$TAR_FILE"
    
    # Set permissions on tar file (required for submission)
    echo "Setting permissions on tar file (chmod go+rx)..."
    chmod go+rx "$TAR_FILE"
    echo ""
    echo "Tarball created successfully!"
    echo "To submit, run: ./scripts/submit.sh"
else
    echo "ERROR: Failed to create tar file!"
    exit 1
fi
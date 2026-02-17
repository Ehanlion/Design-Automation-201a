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

# Run Lab 3 flow, parse summary outputs, and update results_submission.txt.
updateResultsSubmissionFromRun() {
    local run_script="$SCRIPT_DIR/runLab3.sh"
    local results_file="$LAB3_DIR/results_submission.txt"
    local run_log="$SUBMISSIONS_DIR/lab3_run_output.log"
    local run_output run_status
    local total_hpwl best_hpwl no_swaps time_sec time_ms
    local tmp_file

    if [ ! -x "$run_script" ]; then
        echo "ERROR: $run_script not found or not executable."
        exit 1
    fi

    if [ ! -f "$results_file" ]; then
        echo "ERROR: $results_file not found."
        exit 1
    fi

    echo "Running runLab3.sh to refresh results_submission.txt..."
    run_output="$("$run_script" 2>&1)"
    run_status=$?
    printf "%s\n" "$run_output" > "$run_log"

    if [ "$run_status" -ne 0 ]; then
        echo "ERROR: runLab3.sh failed. See log: $run_log"
        exit 1
    fi

    total_hpwl="$(printf "%s\n" "$run_output" | sed -n 's/.*Problem 1 -- Total wirelength of original design:[[:space:]]*\([0-9][0-9]*\).*/\1/p' | tail -n 1)"
    best_hpwl="$(printf "%s\n" "$run_output" | sed -n 's/.*Problem 2 -- Total wirelength AFTER my incremental placement algorithm:[[:space:]]*\([0-9][0-9]*\).*/\1/p' | tail -n 1)"
    no_swaps="$(printf "%s\n" "$run_output" | sed -n 's/.*Problem 2 -- Total number of swaps used:[[:space:]]*\([0-9][0-9]*\).*/\1/p' | tail -n 1)"
    time_sec="$(printf "%s\n" "$run_output" | sed -n 's/.*Problem 2 -- Time taken:[[:space:]]*\([0-9.][0-9.]*\)[[:space:]]*sec.*/\1/p' | tail -n 1)"

    if [ -z "$total_hpwl" ] || [ -z "$best_hpwl" ] || [ -z "$no_swaps" ] || [ -z "$time_sec" ]; then
        echo "ERROR: Could not parse summary values from run output. See log: $run_log"
        exit 1
    fi

    time_ms="$(awk -v sec="$time_sec" 'BEGIN { printf "%.3f", sec * 1000 }')"
    tmp_file="$(mktemp)"

    awk -v total="$total_hpwl" \
        -v best="$best_hpwl" \
        -v swaps="$no_swaps" \
        -v tms="$time_ms" '
        /^[[:space:]]*TOTAL_HPWL:/ { $0 = "    TOTAL_HPWL: " total }
        /^[[:space:]]*BEST_HPWL:/ { $0 = "    BEST_HPWL: " best }
        /^[[:space:]]*NO_SWAPS:/ { $0 = "    NO_SWAPS: " swaps }
        /^[[:space:]]*TIME \(ms\):/ { $0 = "    TIME (ms): " tms }
        { print }
    ' "$results_file" > "$tmp_file"

    mv "$tmp_file" "$results_file"

    echo "Updated results_submission.txt from run output:"
    echo "  TOTAL_HPWL: $total_hpwl"
    echo "  BEST_HPWL: $best_hpwl"
    echo "  NO_SWAPS: $no_swaps"
    echo "  TIME (ms): $time_ms"
    echo "Run output log: $run_log"
}

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

# Auto-refresh results before packaging.
updateResultsSubmissionFromRun

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

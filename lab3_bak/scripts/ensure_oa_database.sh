#!/bin/bash

# Ensure OpenAccess libraries/design are present and in sync with LEF/Verilog/DEF.
# Use --force to always rebuild from source inputs (clean baseline).

set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$LAB3_DIR" || exit 1

FORCE_REBUILD=0
while [[ $# -gt 0 ]]; do
	case "$1" in
		--force)
			FORCE_REBUILD=1
			;;
		*)
			echo "ERROR: Unknown option: $1"
			echo "Usage: $0 [--force]"
			exit 1
			;;
	esac
	shift
done

REQUIRED_FILES=(
	"lab1_setup"
	"NangateOpenCellLibrary.lef"
	"s1196_postrouting.v"
	"s1196_postrouting.def"
)

for required in "${REQUIRED_FILES[@]}"; do
	if [ ! -s "$required" ]; then
		echo "ERROR: Required file '$required' is missing or empty in $LAB3_DIR"
		exit 1
	fi
done

# shellcheck disable=SC1091
source lab1_setup

if command -v sha256sum >/dev/null 2>&1; then
	hash_inputs() {
		sha256sum NangateOpenCellLibrary.lef s1196_postrouting.v s1196_postrouting.def \
			| sha256sum | awk '{print $1}'
	}
elif command -v shasum >/dev/null 2>&1; then
	hash_inputs() {
		shasum -a 256 NangateOpenCellLibrary.lef s1196_postrouting.v s1196_postrouting.def \
			| shasum -a 256 | awk '{print $1}'
	}
else
	echo "ERROR: sha256sum/shasum not found; cannot validate OA input freshness."
	exit 1
fi

STATE_DIR="$LAB3_DIR/.oa_state"
STATE_FILE="$STATE_DIR/input.sha256"
mkdir -p "$STATE_DIR"

CURRENT_HASH="$(hash_inputs)"
NEEDS_REBUILD=$FORCE_REBUILD
REASON=""

if [ "$NEEDS_REBUILD" -eq 0 ]; then
	if [ ! -d "DesignLib" ] || [ ! -f "DesignLib/.oalib" ] || \
		[ ! -d "NangateLib" ] || [ ! -f "NangateLib/.oalib" ] || \
		[ ! -s "lib.defs" ]; then
		NEEDS_REBUILD=1
		REASON="missing OpenAccess libraries/design artifacts"
	fi
fi

if [ "$NEEDS_REBUILD" -eq 0 ]; then
	if [ ! -s "$STATE_FILE" ]; then
		NEEDS_REBUILD=1
		REASON="missing OpenAccess input fingerprint"
	else
		SAVED_HASH="$(cat "$STATE_FILE")"
		if [ "$SAVED_HASH" != "$CURRENT_HASH" ]; then
			NEEDS_REBUILD=1
			REASON="LEF/Verilog/DEF inputs changed"
		fi
	fi
fi

if [ "$NEEDS_REBUILD" -eq 1 ]; then
	if [ "$FORCE_REBUILD" -eq 1 ]; then
		echo "Refreshing OA database to a clean baseline ..."
	else
		echo "Refreshing OA database ($REASON) ..."
	fi
	"$SCRIPT_DIR/setup_oa_database.sh"
	printf "%s\n" "$CURRENT_HASH" > "$STATE_FILE"
else
	echo "OpenAccess database is current."
fi

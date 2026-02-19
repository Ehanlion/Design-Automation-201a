#!/usr/bin/env bash
set -euo pipefail

# Runs both static Lab 4 Part 2 flows:
#   1) Timing-driven placement
#   2) Non-timing-driven placement
#
# Usage:
#   ./scripts/runLab4Part2.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${SCRIPT_DIR}/runLab4Part2TD.sh"
"${SCRIPT_DIR}/runLab4Part2NoTD.sh"

#!/usr/bin/env bash
set -euo pipefail

# Runs both static Part 3 variants:
#   1) No stripes
#   2) With stripes
# Then prints summaries using original Part 1 as baseline.
#
# Usage:
#   ./scripts/runLab4Part3StaticBoth.sh [part1_output_dir] [no_stripes_output_dir] [with_stripes_output_dir]
#
# Example:
#   ./scripts/runLab4Part3StaticBoth.sh
#   ./scripts/runLab4Part3StaticBoth.sh output results/part3_static_no_stripes results/part3_static_with_stripes

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

PART1_OUT="${1:-output}"
NO_STRIPES_OUT="${2:-results/part3_static_no_stripes}"
WITH_STRIPES_OUT="${3:-results/part3_static_with_stripes}"

"${SCRIPT_DIR}/runLab4Part3StaticNoStripes.sh" "$NO_STRIPES_OUT"
"${SCRIPT_DIR}/runLab4Part3StaticWithStripes.sh" "$WITH_STRIPES_OUT"
"${SCRIPT_DIR}/summarizePart3.sh" "$PART1_OUT" "$WITH_STRIPES_OUT" "$NO_STRIPES_OUT"

#!/usr/bin/env bash
set -euo pipefail

# Usage: ./scripts/extract_results.sh <output_dir>
# Parses Innovus output files and prints a concise summary.

OUT="${1:-.}"
DNAME="s1494"

echo "=========================================="
echo "  RESULTS SUMMARY"
echo "  Output dir: $OUT"
echo "=========================================="

# --- Setup timing ---
SETUP_RPT="$OUT/${DNAME}_postrouting_setup.tarpt"
SETUP_SLACK="N/A"
if [ -f "$SETUP_RPT" ]; then
    SETUP_SLACK=$(grep -m1 'Slack Time' "$SETUP_RPT" 2>/dev/null | awk '{print $NF}' || echo "N/A")
    echo "  Setup slack (worst): $SETUP_SLACK"
else
    echo "  Setup timing report: NOT FOUND"
fi

# --- Hold timing ---
HOLD_RPT="$OUT/${DNAME}_postrouting_hold.tarpt"
HOLD_SLACK="N/A"
if [ -f "$HOLD_RPT" ]; then
    HOLD_SLACK=$(grep -m1 'Slack Time' "$HOLD_RPT" 2>/dev/null | awk '{print $NF}' || echo "N/A")
    echo "  Hold  slack (worst): $HOLD_SLACK"
else
    echo "  Hold timing report:  NOT FOUND"
fi

# --- DRC violations ---
DRC_RPT="$OUT/${DNAME}.drc.rpt"
DRC_COUNT="N/A"
if [ -f "$DRC_RPT" ]; then
    NO_VIOLS=$(grep -ci 'No DRC violations' "$DRC_RPT" 2>/dev/null || echo "0")
    if [ "$NO_VIOLS" -gt 0 ] 2>/dev/null; then
        DRC_COUNT=0
        echo "  DRC violations:      0 (CLEAN)"
    else
        DRC_COUNT=$(grep -oP '\d+(?= violation)' "$DRC_RPT" 2>/dev/null | head -1 || echo "?")
        echo "  DRC violations:      $DRC_COUNT"
    fi
else
    echo "  DRC report:          NOT FOUND"
fi

# --- Summary report (area, utilization, etc.) ---
SUMMARY="$OUT/summary.rpt"
if [ -f "$SUMMARY" ]; then
    echo ""
    echo "  --- Area & Utilization ---"
    grep -E '(Total area of Standard cells|Total area of Core|Total area of Chip|Effective Utilization|Instances)' "$SUMMARY" 2>/dev/null | sed 's/^/  /' || true
fi

echo ""
echo "=========================================="
echo "  PASS/FAIL"
echo "=========================================="

PASS=true
if [ "$SETUP_SLACK" != "N/A" ]; then
    is_neg=$(awk "BEGIN { print ($SETUP_SLACK < 0) ? 1 : 0 }" 2>/dev/null || echo "0")
    if [ "$is_neg" = "1" ]; then
        echo "  FAIL: Setup slack is negative ($SETUP_SLACK)"
        PASS=false
    fi
fi

if [ "$HOLD_SLACK" != "N/A" ]; then
    is_neg=$(awk "BEGIN { print ($HOLD_SLACK < 0) ? 1 : 0 }" 2>/dev/null || echo "0")
    if [ "$is_neg" = "1" ]; then
        echo "  FAIL: Hold slack is negative ($HOLD_SLACK)"
        PASS=false
    fi
fi

if [ "$DRC_COUNT" != "0" ] && [ "$DRC_COUNT" != "N/A" ]; then
    echo "  FAIL: DRC violations = $DRC_COUNT"
    PASS=false
fi

if [ "$PASS" = true ]; then
    echo "  >>> ALL CHECKS PASSED <<<"
else
    echo "  >>> SOME CHECKS FAILED <<<"
fi

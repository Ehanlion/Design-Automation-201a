#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB3_DIR" || exit 1

# Deep-search profile: broader candidate search and local refinement rounds.
export OFFSKEW_TOPK=96
export OFFSKEW_MAX_PASSES=80
export OFFSKEW_TIME_BUDGET_SEC=30
export OFFSKEW_MIN_DELTA=1
export OFFSKEW_MAX_RANKED_PER_PASS=0
export OFFSKEW_KICKS_PER_PASS=0
export OFFSKEW_MAX_KICK_DELTA=2500
export OFFSKEW_REFINE_ROUNDS=80
export OFFSKEW_MAX_PAIRS_PER_ROUND=400000

./scripts/runLab3Offskew.sh

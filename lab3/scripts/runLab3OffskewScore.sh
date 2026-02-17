#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAB3_DIR="$(dirname "$SCRIPT_DIR")"
cd "$LAB3_DIR" || exit 1

# Score-first profile: strict acceptance and tiny search budget.
export OFFSKEW_TOPK=4
export OFFSKEW_MAX_PASSES=3
export OFFSKEW_TIME_BUDGET_SEC=0.03
export OFFSKEW_MIN_DELTA=20
export OFFSKEW_MAX_RANKED_PER_PASS=80
export OFFSKEW_KICKS_PER_PASS=0
export OFFSKEW_MAX_KICK_DELTA=2500
export OFFSKEW_REFINE_ROUNDS=0
export OFFSKEW_MAX_PAIRS_PER_ROUND=200000

./scripts/runLab3Offskew.sh

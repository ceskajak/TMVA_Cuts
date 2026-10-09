#!/bin/bash

set -euo pipefail

# Runs tmva_apply for each varset, then ups_fitter once per cut-point of
# every TMVA method present in the resulting tree (Cuts, BDTG, HMatrix,
# Fisher, MLP, SVM - whichever have weight files trained for that varset).
#
# - Cuts: scans TMVACutsScore[0..10], TMVA's own 11 efficiency working points.
# - All other (scalar) methods: scans 11 decile cuts computed from that
#   method's own score distribution in the tree, since e.g. BDTG/Fisher/MLP/SVM
#   scores don't share a common range the way Cuts' [0,1] efficiency does.
#
# Usage: ./runTMVAFitScan.sh [varset ...]
#   With no arguments, scans all known varsets.

VARSETS=("$@")
if [ ${#VARSETS[@]} -eq 0 ]; then
    VARSETS=(c16v m16v c6v m6v c5v m5v c2v m2v)
fi

echo "Building tmva_apply and ups_fitter..."
make bin/tmva_apply bin/ups_fitter

run_fit() {
    local treefile="$1" cutExpr="$2" tag="$3" label="$4"
    echo "--- $label ---"
    # ups_fitter exits nonzero (without crashing) when a cut leaves an empty dataset;
    # don't let that abort the rest of the scan.
    ./bin/ups_fitter "$treefile" "$cutExpr" "$tag" || echo "WARNING: fit skipped/failed for $tag" >&2
}

for varset in "${VARSETS[@]}"; do
    echo "=== varset: $varset ==="
    ./bin/tmva_apply "$varset"

    treefile="dataset/weights/${varset}/tree_TMVA-processed.root"
    if [ ! -f "$treefile" ]; then
        echo "WARNING: $treefile not found, skipping fits for $varset." >&2
        continue
    fi

    mapfile -t branches < <(root -l -b -q "tools/list_score_branches.C(\"$treefile\")" 2>/dev/null | tail -n +2)
    if [ ${#branches[@]} -eq 0 ]; then
        echo "WARNING: no TMVA score branches found in $treefile, skipping $varset." >&2
        continue
    fi

    for branch in "${branches[@]}"; do
        if [ "$branch" = "TMVACutsScore" ]; then
            for idx in $(seq 0 10); do
                cutpoint=$(awk -v i="$idx" 'BEGIN { printf "%.1f", i/10 }')
                tag="${varset}_Cuts_cut${idx}"
                run_fit "$treefile" "TMVACutsScore[$idx]>0.5" "$tag" \
                    "$varset: TMVACutsScore[$idx] (efficiency cut-point $cutpoint)"
            done
            continue
        fi

        method="${branch#TMVA}"
        method="${method%Score}"

        mapfile -t deciles < <(root -l -b -q "tools/score_deciles.C(\"$treefile\",\"$branch\")" 2>/dev/null | tail -n +2)
        if [ ${#deciles[@]} -ne 11 ]; then
            echo "WARNING: could not compute deciles for $branch in $varset, skipping." >&2
            continue
        fi

        for idx in $(seq 0 10); do
            threshold="${deciles[$idx]}"
            tag="${varset}_${method}_cut${idx}"
            run_fit "$treefile" "${branch}>${threshold}" "$tag" \
                "$varset: ${branch} > ${threshold} (decile $idx)"
        done
    done
done

echo "Finished!"

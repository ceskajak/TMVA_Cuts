#!/bin/bash

set -euo pipefail

echo "Building tmva_apply..."
make bin/tmva_apply

echo "Executing bin/tmva_apply..."
if [ -z "${1:-}" ]; then
    echo "No variable set provided, using default m2v..."
    ./bin/tmva_apply
else
    ./bin/tmva_apply "$1"
fi

echo "Executed!"

echo "Finished!"
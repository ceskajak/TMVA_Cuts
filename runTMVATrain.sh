#!/bin/bash

set -euo pipefail

echo "Building tmva_train..."
make bin/tmva_train

echo "Executing bin/tmva_train..."
./bin/tmva_train

echo "Executed!"

echo "Finished!"
#!/bin/bash

set -euo pipefail

echo "Building ups_fitter..."
make bin/ups_fitter

echo "Executing bin/ups_fitter..."
./bin/ups_fitter

echo "Executed!"

echo "Finished!"
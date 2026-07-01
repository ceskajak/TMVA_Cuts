#!/bin/bash

echo "Executing src/TMVAApply.cxx..."

root -l -b -q "src/TMVAApply.cxx+(\"$1\")"

echo "Executed!"
echo "Cleanup..."

rm -f src/*.pcm
rm -f src/*.d
rm -f src/*.so
rm -f src/*TMVAApply*.h

echo "Finished!"
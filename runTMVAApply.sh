#!/bin/bash

echo "Executing src/TMVAApply.cxx..."

root -l -b -q "src/TMVAApply.cxx+(\"$1\")"

echo "Executed!"
echo "Cleanup..."

rm src/*.pcm
rm src/*.d
rm src/*.so
rm src/*TMVAApply*.h

echo "Finished!"
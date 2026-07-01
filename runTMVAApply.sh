#!/bin/bash

echo "Executing src/TMVAApply.cxx..."
if [ -z "$1" ]; then
    echo "No varible set provided, using default..."
    root -l -b -q "src/TMVAApply.cxx+()"
else
    root -l -b -q "src/TMVAApply.cxx+(\"$1\")"
fi

echo "Executed!"
echo "Cleanup..."

rm -f src/*.pcm
rm -f src/*.d
rm -f src/*.so
rm -f src/*TMVAApply*.h

echo "Finished!"
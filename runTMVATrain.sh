#!/bin/bash

echo "Executing src/TMVATrain.cxx..."

root -l -b -q src/TMVATrain.cxx+

echo "Executed!"
echo "Cleanup..."

rm -f src/*.pcm
rm -f src/*.d
rm -f src/*.so
rm -f src/*TMVATrain*.h

echo "Finished!"
#!/bin/bash

echo "Executing src/TMVATrain.cxx..."
root -l -b -q src/TMVATrain.cxx+
echo "Executed!"
echo "Cleanup..."
rm src/*.pcm
rm src/*.d
rm src/*.so
echo "Finished!"
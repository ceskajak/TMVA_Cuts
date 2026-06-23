#!/bin/bash

echo "Executing src/TMVAApply.cxx..."
root -l -b -q src/TMVAApply.cxx+
echo "Executed!"
echo "Cleanup..."
rm src/*.pcm
rm src/*.d
rm src/*.so
echo "Finished!"
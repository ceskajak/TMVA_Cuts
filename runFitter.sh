#!/bin/bash

echo "Executing src/UpsFitter.cxx..."

root -l -b -q src/UpsFitter.cxx+

echo "Executed!"
echo "Cleanup..."

rm src/*.pcm
rm src/*.d
rm src/*.so
rm src/*UpsFitter*.h

echo "Finished!"
#!/bin/bash

echo "Executing src/UpsFitter.cxx..."

root -l -b -q src/UpsFitter.cxx+

echo "Executed!"
echo "Cleanup..."

rm -f src/*.pcm
rm -f src/*.d
rm -f src/*.so
rm -f src/*UpsFitter*.h

echo "Finished!"
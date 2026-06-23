#!/bin/bash

# Script to run UpsFitter.cxx for a grid of DZ and Dphi values.
# Uses up to 6 parallel ROOT jobs by default.

parallel_jobs=6

echo "Starting UpsFitter grid run for DZ=5.0..10.0 and Dphi=0.020..0.050"

for dz_i in {50..100..5}; do
    dz=$(printf "%.1f" "$(echo "$dz_i / 10" | bc -l)")
    for dphi_i in {20..50..2}; do
        dphi=$(printf "0.%03d" "$dphi_i")
        echo "Submitting DZ=${dz}, Dphi=${dphi}"
        root -l -b -q "UpsFitter.cxx(${dz},${dphi})" > /dev/null 2>&1 &
        while [ "$(jobs -rp | wc -l)" -ge "$parallel_jobs" ]; do
            sleep 2
        done
    done
done

wait

echo "All UpsFitter jobs completed!"

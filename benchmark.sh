#!/bin/bash

for i in {1..12}; do
    mpirun -np $i ./main 100000000
done

echo "Benchmark finished."
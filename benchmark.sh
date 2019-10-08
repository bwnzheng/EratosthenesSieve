#!/bin/bash

echo -e "p\tn\tt"

for i in {1..10}; do
  for j in {100,10000,1000000,100000000}; do
    mpirun -np $i ./main $j
  done
done

echo "Benchmark finished."
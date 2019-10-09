#!/bin/bash

#echo -e "p\tn\tt"
for i in {1..12}; do
  for j in {100,1000,10000,100000,1000000,10000000,100000000}; do
    mpirun -np $i ./main $j
  done
done

echo "Benchmark finished."
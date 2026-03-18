#!/bin/bash

N_VALUES=(256 512 1024 2048 4096 8192)

for N in "${N_VALUES[@]}"; do
    echo "Generating N=$N"
    ./make_matrix A_$N.bin $N $N
    ./make_matrix X_$N.bin $N 1
done
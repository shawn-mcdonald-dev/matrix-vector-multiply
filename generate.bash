#!/bin/bash
#SBATCH --job-name=gen_matrices
#SBATCH --output=gen_matrices.out
#SBATCH --partition=shared
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=00:30:00
#SBATCH --account=ccu108

module purge
module load cpu
module load gcc/10.2.0

N_VALUES=(256 512 1024 2048 4096 8192)

for N in "${N_VALUES[@]}"; do
    echo "Generating N=$N"

    # Use deterministic but different seeds
    ./make_matrix A_${N}.bin $N $N $((N))
    ./make_matrix B_${N}.bin $N $N $((N + 1000))

done

echo "All matrices generated."
#!/bin/bash
#SBATCH --job-name=omp_p32
#SBATCH --output=omp_p32.%j.out
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --mem=20GB
#SBATCH --account=ccu108
#SBATCH --time=02:00:00

module purge
module load cpu
module load gcc/10.2.0

P=32

N_VALUES=(256 512 1024 2048 4096 8192)

echo "N,P,Tp_overall,Tp_work" > omp_results_p${P}.csv

for N in "${N_VALUES[@]}"; do

    OUTPUT=$(./omp_matrix_matrix A_${N}.bin B_${N}.bin C.bin $P)

    TP_OVERALL=$(echo "$OUTPUT" | grep "Overall" | awk '{print $3}')
    TP_WORK=$(echo "$OUTPUT" | grep "Compute" | awk '{print $3}')

    echo "$N,$P,$TP_OVERALL,$TP_WORK" >> omp_results_p${P}.csv

done

echo "Done P=$P"

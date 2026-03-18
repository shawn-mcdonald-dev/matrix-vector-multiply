#!/bin/bash
#SBATCH --job-name=mv_p8
#SBATCH --output=mv_p8.%j.out
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=8
#SBATCH --mem=20GB
#SBATCH --account=ccu108
#SBATCH --time=02:00:00

module purge
module load cpu
module load gcc/10.2.0

P=8

N_VALUES=(256 512 1024 2048 4096 8192)

echo "N,P,Tp_overall,Tp_work" > results_p${P}.csv

for N in "${N_VALUES[@]}"; do

    OUTPUT=$(./pth_matrix_matrix A_${N}.bin B_${N}.bin C.bin $P)

    TP_OVERALL=$(echo "$OUTPUT" | grep "Overall" | awk '{print $3}')
    TP_WORK=$(echo "$OUTPUT" | grep "Compute" | awk '{print $3}')

    echo "$N,$P,$TP_OVERALL,$TP_WORK" >> results_p${P}.csv

done
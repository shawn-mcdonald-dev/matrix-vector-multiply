#!/bin/bash
#SBATCH --job-name=mv_p1_1024
#SBATCH --output=mv_p1_1024.%j.out
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --mem=20GB
#SBATCH --account=ccu108
#SBATCH --time=02:00:00

module purge
module load cpu
module load gcc/10.2.0

OUTPUT=$(./pth_matrix_matrix A_1024.bin B_1024.bin C.bin 1)

TP_OVERALL=$(echo "$OUTPUT" | grep "Overall" | awk '{print $3}')
TP_WORK=$(echo "$OUTPUT" | grep "Compute" | awk '{print $3}')

echo "$TP_OVERALL"
echo "$TP_WORK"
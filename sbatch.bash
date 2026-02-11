#!/bin/bash
#SBATCH --job-name=matrix_vector_scaling
#SBATCH --output=matrix_vector_scaling.%j.out
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=128
#SBATCH --mem=20GB
#SBATCH --account=ccu108
#SBATCH --time=02:00:00

module purge
module load cpu
module load slurm
module load gcc/10.2.0

# ===============================
# USER CONFIGURATION
# ===============================

DEBUG=0      # 1 = debug mode, 0 = final paper mode
NUM_RUNS=3   # Averaging count

if [ "$DEBUG" -eq 1 ]; then
    N_VALUES=(1000 2000 4000)
    P_VALUES=(1 2 4)
else
    N_VALUES=(10000 20000 30000 40000)
    P_VALUES=(1 2 4 8 16 32 64 128)
fi

RESULT_FILE="results.csv"

echo "N,P,run,overall_time,work_time" > $RESULT_FILE

# ===============================
# BEGIN EXPERIMENTS
# ===============================

for N in "${N_VALUES[@]}"; do

    echo "Generating matrix A and vector X for N=$N"

    ./make_matrix A.bin $N $N
    ./make_matrix X.bin $N 1

    for P in "${P_VALUES[@]}"; do

        for ((run=1; run<=NUM_RUNS; run++)); do

            echo "Running N=$N P=$P Run=$run"

            OUTPUT=$(./pth_matrix_vector A.bin X.bin Y.bin $P)

            OVERALL=$(echo "$OUTPUT" | grep "Overall time" | awk '{print $3}')
            WORK=$(echo "$OUTPUT" | grep "Compute time" | awk '{print $3}')

            echo "$N,$P,$run,$OVERALL,$WORK" >> $RESULT_FILE

        done
    done

    rm -f A.bin X.bin Y.bin

done

echo "Experiments complete."

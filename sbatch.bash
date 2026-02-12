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
# CONFIGURATION
# ===============================

DEBUG=0   # 1 = small quick test, 0 = full experiment

if [ "$DEBUG" -eq 1 ]; then
    N_VALUES=(1000 2000)
    P_VALUES=(1 2 4)
else
    N_VALUES=(10000 20000 30000 40000)
    P_VALUES=(1 2 4 8 16 32 64 128)
fi

RESULT_FILE="results.csv"
echo "N,P,Tp,Speedup,Efficiency" > $RESULT_FILE

# ===============================
# EXPERIMENT LOOP
# ===============================

for N in "${N_VALUES[@]}"; do

    echo "Generating matrix for N=$N"
    ./make_matrix A.bin $N $N
    ./make_matrix X.bin $N 1

    T1=0

    for P in "${P_VALUES[@]}"; do

        echo "Running N=$N P=$P"

        OUTPUT=$(./pth_matrix_vector A.bin X.bin Y.bin $P)

        TP=$(echo "$OUTPUT" | grep "Overall time" | awk '{print $3}')

        # Ensure 6 decimal precision
        TP=$(printf "%.6f" $TP)

        if [ "$P" -eq 1 ]; then
            T1=$TP
            SPEEDUP=1.000000
            EFFICIENCY=1.000000
        else
            SPEEDUP=$(awk "BEGIN {printf \"%.6f\", $T1 / $TP}")
            EFFICIENCY=$(awk "BEGIN {printf \"%.6f\", $SPEEDUP / $P}")
        fi

        echo "$N,$P,$TP,$SPEEDUP,$EFFICIENCY" >> $RESULT_FILE

    done

    rm -f A.bin X.bin Y.bin

done

echo "All experiments complete."

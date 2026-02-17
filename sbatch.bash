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

DEBUG=1   # 1 = quick test, 0 = full experiment

if [ "$DEBUG" -eq 1 ]; then
    N_VALUES=(1000 2000 4000)
    P_VALUES=(1 2 4)
else
    N_VALUES=(10000 20000 30000 40000)
    P_VALUES=(1 2 4 8 16 32 64 128)
fi

RESULT_FILE="results.csv"

echo "N,P,Tp_overall,Speedup_overall,Efficiency_overall,Tp_work,Speedup_work,Efficiency_work" > $RESULT_FILE

# ===============================
# EXPERIMENT LOOP
# ===============================

for N in "${N_VALUES[@]}"; do

    echo "Generating matrix for N=$N"
    ./make_matrix A.bin $N $N
    ./make_matrix X.bin $N 1

    T1_overall=0
    T1_work=0

    for P in "${P_VALUES[@]}"; do

        echo "Running N=$N P=$P"

        OUTPUT=$(./pth_matrix_vector_numa A.bin X.bin Y.bin $P)

        TP_OVERALL=$(echo "$OUTPUT" | grep "Overall time" | awk '{print $3}')
        TP_WORK=$(echo "$OUTPUT" | grep "Compute time" | awk '{print $3}')

        TP_OVERALL=$(printf "%.6f" $TP_OVERALL)
        TP_WORK=$(printf "%.6f" $TP_WORK)

        if [ "$P" -eq 1 ]; then
            T1_overall=$TP_OVERALL
            T1_work=$TP_WORK

            SPEEDUP_overall=1.000000
            EFFICIENCY_overall=1.000000

            SPEEDUP_work=1.000000
            EFFICIENCY_work=1.000000
        else
            SPEEDUP_overall=$(awk "BEGIN {printf \"%.6f\", $T1_overall / $TP_OVERALL}")
            EFFICIENCY_overall=$(awk "BEGIN {printf \"%.6f\", $SPEEDUP_overall / $P}")

            SPEEDUP_work=$(awk "BEGIN {printf \"%.6f\", $T1_work / $TP_WORK}")
            EFFICIENCY_work=$(awk "BEGIN {printf \"%.6f\", $SPEEDUP_work / $P}")
        fi

        echo "$N,$P,$TP_OVERALL,$SPEEDUP_overall,$EFFICIENCY_overall,$TP_WORK,$SPEEDUP_work,$EFFICIENCY_work" >> $RESULT_FILE

    done

    rm -f A.bin X.bin Y.bin

done

echo "All experiments complete."

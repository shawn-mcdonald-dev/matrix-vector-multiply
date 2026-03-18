#!/bin/bash
#SBATCH --job-name=mv_p2
#SBATCH --output=mv_p2.%j.out
#SBATCH --partition=compute
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=2
#SBATCH --mem=20GB
#SBATCH --account=ccu108
#SBATCH --time=02:00:00

module purge
module load cpu
module load gcc/10.2.0

P=2

N_VALUES=(256 512 1024 2048 4096 8192)

RESULT_FILE="results_p${P}.csv"

echo "N,P,Tp_overall,Speedup_overall,Efficiency_overall,Tp_work,Speedup_work,Efficiency_work" > $RESULT_FILE

T1_overall=0
T1_work=0

for N in "${N_VALUES[@]}"; do

    echo "Running N=$N P=$P"

    OUTPUT=$(./pth_matrix_vector A_${N}.bin X_${N}.bin Y.bin $P)

    TP_OVERALL=$(echo "$OUTPUT" | grep "Overall time" | awk '{print $3}')
    TP_WORK=$(echo "$OUTPUT" | grep "Compute time" | awk '{print $3}')

    TP_OVERALL=$(printf "%.6f" $TP_OVERALL)
    TP_WORK=$(printf "%.6f" $TP_WORK)

    if [ "$P" -eq 1 ]; then
        SPEEDUP_overall=1.0
        EFFICIENCY_overall=1.0
        SPEEDUP_work=1.0
        EFFICIENCY_work=1.0

        T1_overall=$TP_OVERALL
        T1_work=$TP_WORK
    else
        # NOTE: You’ll merge with P=1 later
        SPEEDUP_overall=0
        EFFICIENCY_overall=0
        SPEEDUP_work=0
        EFFICIENCY_work=0
    fi

    echo "$N,$P,$TP_OVERALL,$SPEEDUP_overall,$EFFICIENCY_overall,$TP_WORK,$SPEEDUP_work,$EFFICIENCY_work" >> $RESULT_FILE

done

echo "Done P=$P"
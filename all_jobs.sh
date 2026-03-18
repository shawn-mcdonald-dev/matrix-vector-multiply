for P in 1 2 4 8 16 32 64 128; do
    sbatch run_p${P}.sh
done
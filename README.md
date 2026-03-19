# matrix-vector-multiply

## Getting Started
To follow experiment results:

1. Compile
```bash
make
```

2. Generate all matrices
```bash
sbatch generate.bash
```

3. Run all pthreads jobs (p1-p128)
```bash
bash all_jobs.sh
```

4. Run all OpenMP jobs (p1-p128)
```bash
bash all_jobs_omp.sh
```

## Visualization
To visualize the matrix-matrix results:

1. Create a venv and install requirements.txt
```bash
pip install -r requirements.txt
```

2. Run the following script:
```bash
python plot_matrix_matrix.py
```
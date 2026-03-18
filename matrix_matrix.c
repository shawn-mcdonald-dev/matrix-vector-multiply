/* File:     matrix_matrix.c
 *
 * Purpose:  Serial matrix-matrix multiplication using binary files.
 *
 * Compile:  gcc -std=c99 -Wall -Wextra -O2 -o matrix_matrix matrix_matrix.c
 * Run:      ./matrix_matrix <file A> <file B> <file C>
 *
 * Input files:
 *   - file A: binary matrix file containing an m x k matrix
 *   - file B: binary matrix file containing a k x n matrix
 * Output file:
 *   - file C: binary matrix file containing the result C = A * B (m x n)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* Timing macro (same as in matrix_vector.c) */
#define GET_TIME(now) {                 \
    struct timeval t;                   \
    gettimeofday(&t, NULL);             \
    now = t.tv_sec + t.tv_usec / 1e6;   \
}

int main(int argc, char *argv[]) {
    double overall_start, overall_end;
    double compute_start, compute_end;
    GET_TIME(overall_start);

    FILE *fa = NULL, *fb = NULL, *fc = NULL;
    int A_rows, A_cols;
    int B_rows, B_cols;
    double *A = NULL, *B = NULL, *C = NULL;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file A> <file B> <file C>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open input matrices */
    fa = fopen(argv[1], "rb");
    if (fa == NULL) {
        perror("Error opening matrix A");
        exit(EXIT_FAILURE);
    }

    fb = fopen(argv[2], "rb");
    if (fb == NULL) {
        perror("Error opening matrix B");
        fclose(fa);
        exit(EXIT_FAILURE);
    }

    /* Read dimensions */
    if (fread(&A_rows, sizeof(int), 1, fa) != 1 ||
        fread(&A_cols, sizeof(int), 1, fa) != 1) {
        fprintf(stderr, "Error reading dimensions for matrix A\n");
        fclose(fa);
        fclose(fb);
        exit(EXIT_FAILURE);
    }

    if (fread(&B_rows, sizeof(int), 1, fb) != 1 ||
        fread(&B_cols, sizeof(int), 1, fb) != 1) {
        fprintf(stderr, "Error reading dimensions for matrix B\n");
        fclose(fa);
        fclose(fb);
        exit(EXIT_FAILURE);
    }

    /* Validate multiplication compatibility: A_cols must equal B_rows */
    if (A_cols != B_rows) {
        fprintf(stderr,
                "Error: incompatible dimensions for multiplication\n"
                "Matrix A is %d x %d, matrix B is %d x %d\n",
                A_rows, A_cols, B_rows, B_cols);
        fclose(fa);
        fclose(fb);
        exit(EXIT_FAILURE);
    }

    /* Allocate memory for A, B, C */
    A = malloc((size_t)A_rows * (size_t)A_cols * sizeof(double));
    B = malloc((size_t)B_rows * (size_t)B_cols * sizeof(double));
    C = calloc((size_t)A_rows * (size_t)B_cols, sizeof(double));

    if (A == NULL || B == NULL || C == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        fclose(fa);
        fclose(fb);
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    /* Read matrix data */
    if (fread(A, sizeof(double), (size_t)A_rows * (size_t)A_cols, fa)
            != (size_t)A_rows * (size_t)A_cols) {
        fprintf(stderr, "Error reading data for matrix A\n");
        fclose(fa);
        fclose(fb);
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    if (fread(B, sizeof(double), (size_t)B_rows * (size_t)B_cols, fb)
            != (size_t)B_rows * (size_t)B_cols) {
        fprintf(stderr, "Error reading data for matrix B\n");
        fclose(fa);
        fclose(fb);
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    fclose(fa);
    fclose(fb);

    /* Matrix-matrix multiplication: C = A * B
     * A is A_rows x A_cols
     * B is B_rows x B_cols (with B_rows == A_cols)
     * C is A_rows x B_cols
     */
    GET_TIME(compute_start);
    for (int i = 0; i < A_rows; i++) {
        for (int j = 0; j < B_cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < A_cols; k++) {
                sum += A[i * A_cols + k] * B[k * B_cols + j];
            }
            C[i * B_cols + j] = sum;
        }
    }
    GET_TIME(compute_end);

    /* Write output matrix C */
    fc = fopen(argv[3], "wb");
    if (fc == NULL) {
        perror("Error opening output matrix C");
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    if (fwrite(&A_rows, sizeof(int), 1, fc) != 1 ||
        fwrite(&B_cols, sizeof(int), 1, fc) != 1) {
        fprintf(stderr, "Error writing dimensions for matrix C\n");
        fclose(fc);
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    if (fwrite(C, sizeof(double), (size_t)A_rows * (size_t)B_cols, fc)
            != (size_t)A_rows * (size_t)B_cols) {
        fprintf(stderr, "Error writing data for matrix C\n");
        fclose(fc);
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    fclose(fc);

    free(A);
    free(B);
    free(C);

    GET_TIME(overall_end);
    printf("Overall time: %f seconds\n", overall_end - overall_start);
    printf("Compute time: %f seconds\n", compute_end - compute_start);

    return 0;
}


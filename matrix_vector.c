/* File:     matrix_vector.c
 *
 * Purpose:  Serial matrix-vector multiplication using binary files.
 *
 * Compile:  gcc -std=c99 -Wall -Wextra -o matrix_vector matrix_vector.c
 * Run:      ./matrix_vector <file A> <file x> <file y>
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* Timing macro */
#define GET_TIME(now) {                 \
    struct timeval t;                  \
    gettimeofday(&t, NULL);             \
    now = t.tv_sec + t.tv_usec / 1e6;   \
}

int main(int argc, char* argv[]) {
    double overall_start, overall_end;
    double compute_start, compute_end;
    GET_TIME(overall_start);

    FILE *fa, *fx, *fy;
    int A_rows, A_cols;
    int x_rows, x_cols;
    double *A = NULL, *x = NULL, *y = NULL;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file A> <file x> <file y>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open input files */
    fa = fopen(argv[1], "rb");
    if (fa == NULL) {
        perror("Error opening matrix A");
        exit(EXIT_FAILURE);
    }

    fx = fopen(argv[2], "rb");
    if (fx == NULL) {
        perror("Error opening vector x");
        fclose(fa);
        exit(EXIT_FAILURE);
    }

    /* Read dimensions */
    fread(&A_rows, sizeof(int), 1, fa);
    fread(&A_cols, sizeof(int), 1, fa);

    fread(&x_rows, sizeof(int), 1, fx);
    fread(&x_cols, sizeof(int), 1, fx);

    /* Validate vector */
    if (x_cols != 1) {
        fprintf(stderr,
                "Error: x must be a column vector (cols == 1). Found cols = %d\n",
                x_cols);
        fclose(fa);
        fclose(fx);
        exit(EXIT_FAILURE);
    }

    /* Validate multiplication compatibility */
    if (A_cols != x_rows) {
        fprintf(stderr,
                "Error: incompatible dimensions for multiplication\n"
                "Matrix A is %d x %d, vector x is %d x %d\n",
                A_rows, A_cols, x_rows, x_cols);
        fclose(fa);
        fclose(fx);
        exit(EXIT_FAILURE);
    }

    /* Allocate memory */
    A = malloc(A_rows * A_cols * sizeof(double));
    x = malloc(x_rows * sizeof(double));
    y = calloc(A_rows, sizeof(double));

    if (A == NULL || x == NULL || y == NULL) {
        fprintf(stderr, "Error: memory allocation failed\n");
        fclose(fa);
        fclose(fx);
        free(A);
        free(x);
        free(y);
        exit(EXIT_FAILURE);
    }

    /* Read data */
    fread(A, sizeof(double), A_rows * A_cols, fa);
    fread(x, sizeof(double), x_rows, fx);

    fclose(fa);
    fclose(fx);

    GET_TIME(compute_start);
    /* Matrix-vector multiplication */
    for (int i = 0; i < A_rows; i++) {
        for (int j = 0; j < A_cols; j++) {
            y[i] += A[i * A_cols + j] * x[j];
        }
    }
    GET_TIME(compute_end);

    /* Write output vector */
    fy = fopen(argv[3], "wb");
    if (fy == NULL) {
        perror("Error opening output file");
        free(A);
        free(x);
        free(y);
        exit(EXIT_FAILURE);
    }

    fwrite(&A_rows, sizeof(int), 1, fy);
    fwrite(&(int){1}, sizeof(int), 1, fy);
    fwrite(y, sizeof(double), A_rows, fy);

    fclose(fy);

    free(A);
    free(x);
    free(y);

    GET_TIME(overall_end);
    printf("Overall time: %f seconds\n", overall_end - overall_start);
    printf("Compute time: %f seconds\n", compute_end - compute_start);

    return 0;
}

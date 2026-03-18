/* File:     pth_matrix_matrix.c
 *
 * Purpose:  Parallel matrix-matrix multiplication using pthreads
 *           and Quinn block partitioning macros.
 *
 * Usage:    ./pth_matrix_matrix <file A> <file B> <file C> <P>
 *
 *   - file A: binary matrix file containing an m x k matrix
 *   - file B: binary matrix file containing a k x n matrix
 *   - file C: output binary matrix file containing C = A * B (m x n)
 *   - P:      number of threads
 *
 * Both A and B may be non-square as long as their dimensions are
 * compatible for multiplication.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include "quinn.h"

/* Timing macro (same as in matrix_vector.c) */
#define GET_TIME(now) {                 \
    struct timeval t;                   \
    gettimeofday(&t, NULL);             \
    now = t.tv_sec + t.tv_usec / 1e6;   \
}

/* Global shared data */
double *A = NULL;
double *B = NULL;
double *C = NULL;

int A_rows, A_cols;
int B_rows, B_cols;
int thread_count;

/* ============================= */
/* NUMA FIRST-TOUCH FUNCTION     */
/* ============================= */

void* Numa_first_touch(void* rank) {
    long my_rank = (long) rank;

    /* Partition rows of C (and corresponding rows of A) among threads */
    int my_first_row = BLOCK_LOW(my_rank, thread_count, A_rows);
    int my_last_row  = BLOCK_HIGH(my_rank, thread_count, A_rows);

    /* Touch assigned rows of A and C to establish memory locality */
    for (int i = my_first_row; i <= my_last_row; i++) {
        for (int j = 0; j < A_cols; j++) {
            A[i * A_cols + j] = A[i * A_cols + j];
        }
        for (int j = 0; j < B_cols; j++) {
            C[i * B_cols + j] = C[i * B_cols + j];
        }
    }

    /* Touch all of B in a block-cyclic fashion over rows to distribute pages.
     * This is a heuristic; B is read-only and shared by all threads.
     */
    int my_first_b_row = BLOCK_LOW(my_rank, thread_count, B_rows);
    int my_last_b_row  = BLOCK_HIGH(my_rank, thread_count, B_rows);

    for (int i = my_first_b_row; i <= my_last_b_row; i++) {
        for (int j = 0; j < B_cols; j++) {
            B[i * B_cols + j] = B[i * B_cols + j];
        }
    }

    return NULL;
}

/* ============================= */
/* COMPUTE FUNCTION              */
/* ============================= */

void* Mat_mat_mult(void* rank) {
    long my_rank = (long) rank;

    int my_first_row = BLOCK_LOW(my_rank, thread_count, A_rows);
    int my_last_row  = BLOCK_HIGH(my_rank, thread_count, A_rows);

    /* Compute this thread's block of rows of C */
    for (int i = my_first_row; i <= my_last_row; i++) {
        for (int j = 0; j < B_cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < A_cols; k++) {
                sum += A[i * A_cols + k] * B[k * B_cols + j];
            }
            C[i * B_cols + j] = sum;
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    double overall_start, overall_end;
    double compute_start, compute_end;
    GET_TIME(overall_start);

    if (argc != 5) {
        fprintf(stderr,
                "Usage: %s <file A> <file B> <file C> <P>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    thread_count = atoi(argv[4]);
    if (thread_count <= 0) {
        fprintf(stderr, "Error: P must be positive\n");
        exit(EXIT_FAILURE);
    }

    /* Open input matrices */
    FILE *fa = fopen(argv[1], "rb");
    if (fa == NULL) {
        perror("Error opening matrix A");
        exit(EXIT_FAILURE);
    }

    FILE *fb = fopen(argv[2], "rb");
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
    C = malloc((size_t)A_rows * (size_t)B_cols * sizeof(double));

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

    /* ============================= */
    /* NUMA FIRST-TOUCH PHASE        */
    /* ============================= */

    pthread_t* thread_handles =
        malloc((size_t)thread_count * sizeof(pthread_t));
    if (thread_handles == NULL) {
        fprintf(stderr, "Error: memory allocation failed for thread handles\n");
        free(A);
        free(B);
        free(C);
        exit(EXIT_FAILURE);
    }

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread],
                       NULL,
                       Numa_first_touch,
                       (void*) thread);
    }

    for (int thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    /* ============================= */
    /* COMPUTE PHASE                 */
    /* ============================= */

    GET_TIME(compute_start);

    for (long thread = 0; thread < thread_count; thread++) {
        pthread_create(&thread_handles[thread],
                       NULL,
                       Mat_mat_mult,
                       (void*) thread);
    }

    for (int thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    GET_TIME(compute_end);

    /* Write output matrix C */
    FILE* fc = fopen(argv[3], "wb");
    if (fc == NULL) {
        perror("Error opening output matrix C");
        free(A);
        free(B);
        free(C);
        free(thread_handles);
        exit(EXIT_FAILURE);
    }

    if (fwrite(&A_rows, sizeof(int), 1, fc) != 1 ||
        fwrite(&B_cols, sizeof(int), 1, fc) != 1) {
        fprintf(stderr, "Error writing dimensions for matrix C\n");
        fclose(fc);
        free(A);
        free(B);
        free(C);
        free(thread_handles);
        exit(EXIT_FAILURE);
    }

    if (fwrite(C, sizeof(double), (size_t)A_rows * (size_t)B_cols, fc)
            != (size_t)A_rows * (size_t)B_cols) {
        fprintf(stderr, "Error writing data for matrix C\n");
        fclose(fc);
        free(A);
        free(B);
        free(C);
        free(thread_handles);
        exit(EXIT_FAILURE);
    }

    fclose(fc);

    free(A);
    free(B);
    free(C);
    free(thread_handles);

    GET_TIME(overall_end);

    printf("Overall time: %f seconds\n", overall_end - overall_start);
    printf("Compute time: %f seconds\n", compute_end - compute_start);

    return 0;
}


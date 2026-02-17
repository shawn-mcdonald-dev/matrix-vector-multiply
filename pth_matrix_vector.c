/* File:     pth_matrix_vector.c
 *
 * Purpose:  Parallel matrix-vector multiplication using pthreads
 *           and Quinn block partitioning macros.
 *
 * NUMA enhancement: parallel first-touch initialization
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "quinn.h"
#include <sys/time.h>

/* Timing macro */
#define GET_TIME(now) {                 \
    struct timeval t;                  \
    gettimeofday(&t, NULL);             \
    now = t.tv_sec + t.tv_usec / 1e6;   \
}

/* Global shared variables */
double *A = NULL;
double *x = NULL;
double *y = NULL;

int A_rows, A_cols;
int x_rows, x_cols;
int thread_count;

/* ============================= */
/* NUMA FIRST-TOUCH FUNCTION     */
/* ============================= */

void* Numa_first_touch(void* rank) {
    long my_rank = (long) rank;

    int my_first_row = BLOCK_LOW(my_rank, thread_count, A_rows);
    int my_last_row  = BLOCK_HIGH(my_rank, thread_count, A_rows);

    /* Touch assigned rows of A */
    for (int i = my_first_row; i <= my_last_row; i++) {
        for (int j = 0; j < A_cols; j++) {
            A[i * A_cols + j] = A[i * A_cols + j];
        }
    }

    /* Touch corresponding portion of y */
    for (int i = my_first_row; i <= my_last_row; i++) {
        y[i] = y[i];
    }

    /* Touch portion of x */
    int my_first_x = BLOCK_LOW(my_rank, thread_count, x_rows);
    int my_last_x  = BLOCK_HIGH(my_rank, thread_count, x_rows);

    for (int i = my_first_x; i <= my_last_x; i++) {
        x[i] = x[i];
    }

    return NULL;
}

/* ============================= */
/* COMPUTE FUNCTION              */
/* ============================= */

void* Mat_vect_mult(void* rank) {
    long my_rank = (long) rank;

    int my_first_row = BLOCK_LOW(my_rank, thread_count, A_rows);
    int my_last_row  = BLOCK_HIGH(my_rank, thread_count, A_rows);

    for (int i = my_first_row; i <= my_last_row; i++) {
        y[i] = 0.0;
        for (int j = 0; j < A_cols; j++) {
            y[i] += A[i * A_cols + j] * x[j];
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
                "Usage: %s <file A> <file x> <file y> <P>\n",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    thread_count = atoi(argv[4]);
    if (thread_count <= 0) {
        fprintf(stderr, "Error: P must be positive\n");
        exit(EXIT_FAILURE);
    }

    FILE *fa = fopen(argv[1], "rb");
    FILE *fx = fopen(argv[2], "rb");

    fread(&A_rows, sizeof(int), 1, fa);
    fread(&A_cols, sizeof(int), 1, fa);
    fread(&x_rows, sizeof(int), 1, fx);
    fread(&x_cols, sizeof(int), 1, fx);

    if (A_cols != x_rows) {
        fprintf(stderr, "Dimension mismatch\n");
        exit(EXIT_FAILURE);
    }

    A = malloc(A_rows * A_cols * sizeof(double));
    x = malloc(x_rows * sizeof(double));
    y = calloc(A_rows, sizeof(double));

    fread(A, sizeof(double), A_rows * A_cols, fa);
    fread(x, sizeof(double), x_rows, fx);

    fclose(fa);
    fclose(fx);

    /* ============================= */
    /* NUMA FIRST-TOUCH PHASE        */
    /* ============================= */

    pthread_t* thread_handles =
        malloc(thread_count * sizeof(pthread_t));

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
                       Mat_vect_mult,
                       (void*) thread);
    }

    for (int thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    GET_TIME(compute_end);

    FILE* fy = fopen(argv[3], "wb");
    fwrite(&A_rows, sizeof(int), 1, fy);
    fwrite(&(int){1}, sizeof(int), 1, fy);
    fwrite(y, sizeof(double), A_rows, fy);
    fclose(fy);

    free(A);
    free(x);
    free(y);
    free(thread_handles);

    GET_TIME(overall_end);

    printf("Overall time: %f seconds\n", overall_end - overall_start);
    printf("Compute time: %f seconds\n", compute_end - compute_start);

    return 0;
}

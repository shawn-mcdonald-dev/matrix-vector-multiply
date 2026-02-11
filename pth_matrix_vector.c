/* File:     pth_matrix_vector.c
 *
 * Purpose:  Parallel matrix-vector multiplication using pthreads
 *           and Quinn block partitioning macros.
 *
 * Compile:  gcc -std=c99 -Wall -Wextra -O2 -pthread -o pth_matrix_vector pth_matrix_vector.c
 * Run:      ./pth_matrix_vector A X Y2 P
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

/* Thread function */
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
        fprintf(stderr, "Error: P (number of threads) must be positive\n");
        exit(EXIT_FAILURE);
    }

    FILE *fa = fopen(argv[1], "rb");
    if (fa == NULL) {
        perror("Error opening matrix A");
        exit(EXIT_FAILURE);
    }

    FILE *fx = fopen(argv[2], "rb");
    if (fx == NULL) {
        perror("Error opening vector x");
        fclose(fa);
        exit(EXIT_FAILURE);
    }

    fread(&A_rows, sizeof(int), 1, fa);
    fread(&A_cols, sizeof(int), 1, fa);

    fread(&x_rows, sizeof(int), 1, fx);
    fread(&x_cols, sizeof(int), 1, fx);

    if (x_cols != 1) {
        fprintf(stderr,
                "Error: x must be a column vector (cols == 1). Found cols = %d\n",
                x_cols);
        fclose(fa);
        fclose(fx);
        exit(EXIT_FAILURE);
    }

    if (A_cols != x_rows) {
        fprintf(stderr,
                "Error: incompatible dimensions\n"
                "Matrix A is %d x %d\n"
                "Vector x is %d x %d\n",
                A_rows, A_cols, x_rows, x_cols);
        fclose(fa);
        fclose(fx);
        exit(EXIT_FAILURE);
    }

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

    fread(A, sizeof(double), A_rows * A_cols, fa);
    fread(x, sizeof(double), x_rows, fx);

    fclose(fa);
    fclose(fx);

    GET_TIME(compute_start);

    pthread_t* thread_handles =
        malloc(thread_count * sizeof(pthread_t));

    if (thread_handles == NULL) {
        fprintf(stderr, "Error: thread allocation failed\n");
        free(A); free(x); free(y);
        exit(EXIT_FAILURE);
    }

    for (long thread = 0; thread < thread_count; thread++) {
        if (pthread_create(&thread_handles[thread],
                           NULL,
                           Mat_vect_mult,
                           (void*) thread) != 0) {
            fprintf(stderr, "Error creating thread %ld\n", thread);
            free(A); free(x); free(y);
            free(thread_handles);
            exit(EXIT_FAILURE);
        }
    }

    for (int thread = 0; thread < thread_count; thread++) {
        pthread_join(thread_handles[thread], NULL);
    }

    GET_TIME(compute_end);

    FILE* fy = fopen(argv[3], "wb");
    if (fy == NULL) {
        perror("Error opening output file");
        free(A); free(x); free(y);
        free(thread_handles);
        exit(EXIT_FAILURE);
    }

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

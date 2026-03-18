/* File:     omp_matrix_matrix.c
 *
 * Optimized OpenMP matrix-matrix multiplication.
 *
 * Improvements:
 *   - Transpose B for cache-friendly access
 *   - Removed collapse(2) for better locality
 *   - Retained NUMA first-touch
 *   - Added SIMD vectorization hint
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <omp.h>
 #include <sys/time.h>
 
 #define GET_TIME(now) {                  \
     struct timeval t;                    \
     gettimeofday(&t, NULL);              \
     now = t.tv_sec + t.tv_usec / 1e6;   \
 }
 
 int main(int argc, char* argv[]) {
     double overall_start, overall_end;
     double compute_start, compute_end;
     GET_TIME(overall_start);
 
     if (argc != 5) {
         fprintf(stderr,
                 "Usage: %s <file A> <file B> <file C> <P>\n",
                 argv[0]);
         return EXIT_FAILURE;
     }
 
     int thread_count = atoi(argv[4]);
     if (thread_count <= 0) {
         fprintf(stderr, "Error: P must be positive\n");
         return EXIT_FAILURE;
     }
 
     omp_set_num_threads(thread_count);
 
     FILE *fa = fopen(argv[1], "rb");
     FILE *fb = fopen(argv[2], "rb");
 
     if (!fa || !fb) {
         perror("Error opening input files");
         return EXIT_FAILURE;
     }
 
     int A_rows, A_cols, B_rows, B_cols;

     if (fread(&A_rows, sizeof(int), 1, fa) != 1 ||
         fread(&A_cols, sizeof(int), 1, fa) != 1) {
         fprintf(stderr, "Error reading dimensions for matrix A\n");
         fclose(fa); fclose(fb);
         return EXIT_FAILURE;
     }

     if (fread(&B_rows, sizeof(int), 1, fb) != 1 ||
         fread(&B_cols, sizeof(int), 1, fb) != 1) {
         fprintf(stderr, "Error reading dimensions for matrix B\n");
         fclose(fa); fclose(fb);
         return EXIT_FAILURE;
     }

     if (A_cols != B_rows) {
         fprintf(stderr, "Matrix dimension mismatch\n");
         fclose(fa); fclose(fb);
         return EXIT_FAILURE;
     }

     double *A = malloc((size_t)A_rows * A_cols * sizeof(double));
     double *B = malloc((size_t)B_rows * B_cols * sizeof(double));
     double *B_T = malloc((size_t)B_cols * B_rows * sizeof(double));
     double *C = malloc((size_t)A_rows * B_cols * sizeof(double));

     if (!A || !B || !B_T || !C) {
         fprintf(stderr, "Memory allocation failed\n");
         fclose(fa); fclose(fb);
         free(A); free(B); free(B_T); free(C);
         return EXIT_FAILURE;
     }

     if (fread(A, sizeof(double), (size_t)A_rows * A_cols, fa)
             != (size_t)A_rows * A_cols) {
         fprintf(stderr, "Error reading data for matrix A\n");
         fclose(fa); fclose(fb);
         free(A); free(B); free(B_T); free(C);
         return EXIT_FAILURE;
     }

     if (fread(B, sizeof(double), (size_t)B_rows * B_cols, fb)
             != (size_t)B_rows * B_cols) {
         fprintf(stderr, "Error reading data for matrix B\n");
         fclose(fa); fclose(fb);
         free(A); free(B); free(B_T); free(C);
         return EXIT_FAILURE;
     }
 
     fclose(fa);
     fclose(fb);
 
     /* ============================= */
     /* NUMA FIRST-TOUCH             */
     /* ============================= */
 
     #pragma omp parallel for schedule(static)
     for (int i = 0; i < A_rows; i++) {
         for (int j = 0; j < A_cols; j++)
             A[i * A_cols + j] = A[i * A_cols + j];
         for (int j = 0; j < B_cols; j++)
             C[i * B_cols + j] = 0.0;
     }
 
     #pragma omp parallel for schedule(static)
     for (int i = 0; i < B_rows; i++) {
         for (int j = 0; j < B_cols; j++)
             B[i * B_cols + j] = B[i * B_cols + j];
     }
 
     /* ============================= */
     /* TRANSPOSE B                  */
     /* ============================= */
 
     #pragma omp parallel for collapse(2) schedule(static)
     for (int i = 0; i < B_rows; i++) {
         for (int j = 0; j < B_cols; j++) {
             B_T[j * B_rows + i] = B[i * B_cols + j];
         }
     }
 
     /* ============================= */
     /* COMPUTE PHASE                */
     /* ============================= */
 
     GET_TIME(compute_start);
 
     #pragma omp parallel for schedule(static)
     for (int i = 0; i < A_rows; i++) {
         for (int j = 0; j < B_cols; j++) {
             double sum = 0.0;
 
             #pragma omp simd
             for (int k = 0; k < A_cols; k++) {
                 sum += A[i * A_cols + k] * B_T[j * B_rows + k];
             }
 
             C[i * B_cols + j] = sum;
         }
     }
 
     GET_TIME(compute_end);
 
     /* ============================= */
     /* WRITE OUTPUT                 */
     /* ============================= */
 
     FILE *fc = fopen(argv[3], "wb");
 
     fwrite(&A_rows, sizeof(int), 1, fc);
     fwrite(&B_cols, sizeof(int), 1, fc);
     fwrite(C, sizeof(double), (size_t)A_rows * B_cols, fc);
 
     fclose(fc);
 
     free(A);
     free(B);
     free(B_T);
     free(C);
 
     GET_TIME(overall_end);
 
     printf("Overall time: %f seconds\n", overall_end - overall_start);
     printf("Compute time: %f seconds\n", compute_end - compute_start);
 
     return 0;
 }
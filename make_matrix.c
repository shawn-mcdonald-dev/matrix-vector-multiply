/* File:     make_matrix.c
 *
 * Purpose:  Create a binary matrix file with random double values.
 *
 * Compile:  gcc -std=c99 -Wall -Wextra -o make_matrix make_matrix.c
 * Run:      ./make_matrix <file_name> <rows> <cols>
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    FILE* fp;
    int rows, cols;
    double value;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_name> <rows> <cols>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    rows = atoi(argv[2]);
    cols = atoi(argv[3]);

    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Error: rows and cols must be positive integers\n");
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "wb");
    if (fp == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    /* Write matrix dimensions */
    fwrite(&rows, sizeof(int), 1, fp);
    fwrite(&cols, sizeof(int), 1, fp);

    /* Deterministic random values */
    srand(1);
    for (int i = 0; i < rows * cols; i++) {
        value = ((double) rand() / RAND_MAX) * 10.0;
        fwrite(&value, sizeof(double), 1, fp);
    }

    fclose(fp);
    return 0;
}

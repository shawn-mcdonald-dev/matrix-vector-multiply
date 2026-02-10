/* File:     print_matrix.c
 *
 * Purpose:  Print a binary matrix file to stdout.
 *
 * Compile:  gcc -std=c99 -Wall -Wextra -o print_matrix print_matrix.c
 * Run:      ./print_matrix <file_name>
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    FILE* fp;
    int rows, cols;
    double value;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    if (fread(&rows, sizeof(int), 1, fp) != 1 ||
        fread(&cols, sizeof(int), 1, fp) != 1) {
        fprintf(stderr, "Error reading matrix dimensions\n");
        fclose(fp);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (fread(&value, sizeof(double), 1, fp) != 1) {
                fprintf(stderr, "Error reading matrix data\n");
                fclose(fp);
                exit(EXIT_FAILURE);
            }
            printf("%05.2f ", value);
        }
        printf("\n");
    }

    fclose(fp);
    return 0;
}

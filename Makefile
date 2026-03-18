CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2
PFLAGS  = -std=c99 -Wall -Wextra -O2 -pthread
OFLAGS  = -std=c99 -Wall -Wextra -O2 -fopenmp

TARGETS = make_matrix print_matrix matrix_vector matrix_matrix pth_matrix_vector pth_matrix_matrix omp_matrix_matrix

all: $(TARGETS)

make_matrix: make_matrix.c
	$(CC) $(CFLAGS) -o make_matrix make_matrix.c

print_matrix: print_matrix.c
	$(CC) $(CFLAGS) -o print_matrix print_matrix.c

matrix_vector: matrix_vector.c
	$(CC) $(CFLAGS) -o matrix_vector matrix_vector.c

matrix_matrix: matrix_matrix.c
	$(CC) $(CFLAGS) -o matrix_matrix matrix_matrix.c

pth_matrix_vector: pth_matrix_vector.c
	$(CC) $(PFLAGS) -o pth_matrix_vector pth_matrix_vector.c

pth_matrix_matrix: pth_matrix_matrix.c
	$(CC) $(PFLAGS) -o pth_matrix_matrix pth_matrix_matrix.c

omp_matrix_matrix: omp_matrix_matrix.c
	$(CC) $(OFLAGS) -o omp_matrix_matrix omp_matrix_matrix.c

clean:
	rm -f $(TARGETS) *.o

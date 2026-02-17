CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2
PFLAGS  = -std=c99 -Wall -Wextra -O2 -pthread

TARGETS = make_matrix print_matrix matrix_vector pth_matrix_vector pth_matrix_vector_numa

all: $(TARGETS)

make_matrix: make_matrix.c
	$(CC) $(CFLAGS) -o make_matrix make_matrix.c

print_matrix: print_matrix.c
	$(CC) $(CFLAGS) -o print_matrix print_matrix.c

matrix_vector: matrix_vector.c
	$(CC) $(CFLAGS) -o matrix_vector matrix_vector.c

pth_matrix_vector: pth_matrix_vector.c
	$(CC) $(PFLAGS) -o pth_matrix_vector pth_matrix_vector.c

pth_matrix_vector_numa: pth_matrix_vector_numa.c
	$(CC) $(PFLAGS) -o pth_matrix_vector_numa pth_matrix_vector_numa.c

clean:
	rm -f $(TARGETS) *.o

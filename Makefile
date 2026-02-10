CC      = gcc
CFLAGS  = -std=c99 -Wall -Wextra -O2

TARGETS = make_matrix print_matrix matrix_vector

all: $(TARGETS)

make_matrix: make_matrix.c
	$(CC) $(CFLAGS) -o make_matrix make_matrix.c

print_matrix: print_matrix.c
	$(CC) $(CFLAGS) -o print_matrix print_matrix.c

matrix_vector: matrix_vector.c
	$(CC) $(CFLAGS) -o matrix_vector matrix_vector.c

clean:
	rm -f $(TARGETS) *.o

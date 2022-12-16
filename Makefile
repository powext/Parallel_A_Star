CC=gcc-12
MPI=mpicc
CFLAGS=-I. -Wall -fopenmp
DEPS=main.c main.h

p_a_star: $(DEPS)
	$(MPI) -o p_a_star $(DEPS)

.PHONY: clean

clean:
	rm -f *.o

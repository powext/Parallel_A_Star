#include <mpi.h>
#include <stdlib.h>
#include <stdbool.h>
//#include "../include/priority_queue.h"
//#include "../include/parallel_distribution.h"

#define MAX_EXIT_POINTS 9

extern bool DEBUG;

void parallel_init(int* n_chunks, int* world_rank) {
    MPI_Init(NULL, NULL);

    MPI_Comm_size(MPI_COMM_WORLD, n_chunks);
    MPI_Comm_rank(MPI_COMM_WORLD, world_rank);
}

void parallel_finalize(){
    MPI_Finalize();
}

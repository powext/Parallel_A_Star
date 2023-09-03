#include "../include/util.h"
#include <unistd.h>
#include <mpi.h>

void debug(int rank_to_debug) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (world_rank != rank_to_debug) return;
    volatile int i = 0;
    while (0 == i)
        sleep(5);
}

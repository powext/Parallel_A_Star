#include "mpi.h"
#include "../include/print.h"
#include "../include/parallel_distribution.h"

extern int GRID_HEIGHT;
extern int GRID_WIDTH;
extern int DEBUG;

void parallel_root_init(Node* nodes, Node* starting_node, Node* destination_node) {
    MPI_Init(NULL, NULL);

    int n_chunks;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_size(MPI_COMM_WORLD, &n_chunks);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    distribute_work(nodes, starting_node, destination_node, n_chunks, world_rank);

    if (world_rank != 0) return;
}

void parallel_finalize() {
    MPI_Finalize();
}


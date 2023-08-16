#include <printf.h>
#include <math.h>
#include <tic.h>
#include "mpi.h"
#import "stdlib.h"
#include "../include/print.h"
#include "../include/comm.h"

extern int GRID_HEIGHT;
extern int GRID_WIDTH;
extern int DEBUG;

MPI_Datatype create_node_datatype() {
    MPI_Datatype CoordType;
    MPI_Type_contiguous(2, MPI_INT, &CoordType);
    MPI_Type_commit(&CoordType);

    int blockcounts[6] = {1, 1, 1, 1, 1, 1};
    MPI_Datatype types[6] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, CoordType, MPI_INT};
    MPI_Aint offsets[6];

    offsets[0] = offsetof(Node, id);
    offsets[1] = offsetof(Node, distance);
    offsets[2] = offsetof(Node, normal_distance);
    offsets[3] = offsetof(Node, heuristic_distance);
    offsets[4] = offsetof(Node, coordinates);
    offsets[5] = offsetof(Node, type);

    MPI_Datatype mpi_node_type;
    MPI_Type_create_struct(6, blockcounts, offsets, types, &mpi_node_type);
    MPI_Type_commit(&mpi_node_type);

    return mpi_node_type;
}

void distribute_work(Node* nodes, int n_chunks, int world_rank) {
    if (sqrt(n_chunks) != (int) sqrt(n_chunks)) {
        if (world_rank == 0) {
            printf("[ERROR] The number of processes must be a perfect square.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (GRID_WIDTH * GRID_HEIGHT % n_chunks != 0) {
        if (world_rank == 0) {
            printf("[ERROR] The total size of the array must be a multiple of the number of processes.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (DEBUG && world_rank == 0) {
        printf("[DEBUG] R0 - Nodes:");
        for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; i++)
            printf("%d ", nodes[i].id);
    }

    int n_chunks_per_s = (int) sqrt(n_chunks);
    int chunk_size = GRID_WIDTH * GRID_HEIGHT;
    int chunk_s_length = (int) sqrt((double) chunk_size / n_chunks);
    if (chunk_s_length * chunk_s_length * n_chunks != GRID_WIDTH * GRID_HEIGHT) {
        if (world_rank == 0) {
            printf("[ERROR] The total size of the array must be a perfect square multiple of the number of processes.\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (DEBUG && world_rank == 0) {
        printf("[DEBUG] R0 - chunk_s_length: %d\n", chunk_s_length);
        printf("[DEBUG] R0 - n_chunks: %d\n", n_chunks);
    }

    int* displacements = malloc(sizeof(int) * n_chunks);
    for (int j=0; j < n_chunks_per_s; j++) {
        for (int i=0; i < n_chunks_per_s; i++) {
            displacements[j * n_chunks_per_s + i] = (j * n_chunks_per_s * chunk_s_length) + i;
        }
    }

    if (DEBUG && world_rank == 0) {
        printf("[DEBUG] R0 - Displacements: ");
        for (int i = 0; i < n_chunks; i++) {
            printf("%d ", displacements[i]);
        }
        printf("\n");
    }

    int* send_count = malloc(sizeof(int) * n_chunks);
    for (int i = 0; i < n_chunks; i++)
        send_count[i] = 1;

    MPI_Datatype node_type = create_node_datatype();
    MPI_Datatype node_vector_type, node_vector_type_resized;
    MPI_Type_vector(chunk_s_length, chunk_s_length, GRID_WIDTH, node_type, &node_vector_type);
    MPI_Type_commit(&node_vector_type);
    MPI_Type_create_resized(node_vector_type, 0, chunk_s_length * sizeof(Node), &node_vector_type_resized);
    MPI_Type_commit(&node_vector_type_resized);

    // Scatter the data
    Node* l_nodes = malloc(sizeof(Node) * chunk_s_length * chunk_s_length);
    if (MPI_Scatterv(
            nodes,
            send_count,
            displacements,
            node_vector_type_resized,
            l_nodes,
            chunk_size,
            node_type,
            0,
            MPI_COMM_WORLD)) {
        printf("Error: Scatter error\n");
        exit(1);
    }

    if (DEBUG)
        for (int i = 0; i < n_chunks; i++) {
            if (world_rank == i) {
                printf("Rank %d\n", world_rank);
                for (int j = 0; j < chunk_s_length; j++) {
                    for (int k = 0; k < chunk_s_length; k++) {
                        printf("%d:%d", l_nodes[j * chunk_s_length + k].coordinates.x, l_nodes[j * chunk_s_length + k].coordinates.y);
                    }
                    printf("\n");
                }
                printf("\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }

    free(l_nodes);
    free(displacements);
    free(send_count);
    MPI_Type_free(&node_type);
    MPI_Type_free(&node_vector_type);
    MPI_Type_free(&node_vector_type_resized);
}

void parallel_root_init(Node* nodes) {
    MPI_Init(NULL, NULL);

    int n_chunks;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_size(MPI_COMM_WORLD, &n_chunks);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    distribute_work(nodes, n_chunks, world_rank);

    if (world_rank != 0) return;
}

void parallel_finalize() {
    MPI_Finalize();
}


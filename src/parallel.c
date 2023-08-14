#include <printf.h>
#include "mpi.h"
#include "../include/node.h"
#import "stdlib.h"
#include "../include/print.h"
#include "../include/comm.h"
#include </usr/local/Cellar/libomp/16.0.6/include/omp.h>
#define MAX_EXIT_POINTS 9
#define MAX_PATH_NUM 72

extern int HEIGHT;
extern int WIDTH;
MPI_Datatype create_node_datatype() {
    int blockcounts[7] = {1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype types[7] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[7];

    offsets[0] = offsetof(Node, id);
    offsets[1] = offsetof(Node, distance);
    offsets[2] = offsetof(Node, normal_distance);
    offsets[3] = offsetof(Node, heuristic_distance);
    offsets[4] = offsetof(Node, coordinates.x);
    offsets[5] = offsetof(Node, coordinates.y);
    offsets[6] = offsetof(Node, type);

    MPI_Datatype mpi_node_type;
    MPI_Type_create_struct(7, blockcounts, offsets, types, &mpi_node_type);
    MPI_Type_commit(&mpi_node_type);

    return mpi_node_type;
}

void distribute_work(Node* nodes_old, int n_chunks, int world_rank) {
    Node** nodes = malloc(sizeof(Node*) * 20*20);

    if (world_rank == 0) {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                nodes[(HEIGHT * i) + j] = &nodes_old[(HEIGHT * i) + j];
            }
            printf("\n");
        }
    }

    // Calculate the extent of each block in the original matrix
    int sizes[2] = {20, 20};
    int subsizes[2] = {10, 5};
    int starts[2] = {0, 0};

    MPI_Datatype mpi_node_type = create_node_datatype();
    MPI_Datatype mpi_contig_node_type;
    MPI_Type_contiguous(1, mpi_node_type, &mpi_contig_node_type);
    MPI_Type_commit(&mpi_contig_node_type);

    // Create the subarray data type for the local_nodes
    MPI_Datatype subarray_type;
    MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, mpi_contig_node_type, &subarray_type);
    MPI_Type_commit(&subarray_type);

    Node* local_nodes = (Node*)malloc(sizeof(Node) * 10 * 10);

    if (world_rank == 0) {
        MPI_Send(nodes, 1, subarray_type, 1, 0, MPI_COMM_WORLD);
    } else if (world_rank == 1) {
        printf("Rank %d receiving\n", world_rank);
        MPI_Recv(local_nodes, 1, subarray_type, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        printf("Rank %d received\n", world_rank);
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < 10; j++) {
                printf_with_colors(local_nodes[(i * 10) + j]);
            }
            printf("\n");
        }
        printf("\n");
    }

    MPI_Type_free(&mpi_node_type);
    MPI_Type_free(&mpi_contig_node_type);
    MPI_Type_free(&subarray_type);
    free(local_nodes);
}

void parallel_root_init(Node* nodes) {
    MPI_Init(NULL, NULL);

    int world_size;
    int world_rank;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Get_processor_name(processor_name, &name_len);

    distribute_work(nodes, world_size, world_rank);

    if (world_rank != 0) return;
}

void parallel_finalize() {
    MPI_Finalize();
}

ChunkPath* compute_path(Node** matrix, int width, int heigth, Coordinates start, Coordinates end){
    ChunkPath* tmp_path = malloc(sizeof(ChunkPath));
    // TODO: compute single path
    return tmp_path;
}

MsgChunkEnd* parallel_compute_paths(MsgChunkStart msg){
    MsgChunkEnd* paths_found = malloc(sizeof(MsgChunkEnd));
    paths_found->paths = malloc(MAX_PATH_NUM*sizeof(ChunkPath));
    for (int i = 0; i < MAX_PATH_NUM; i++){
        paths_found->paths[i] = *(ChunkPath*) NULL;
    }

    if (msg.starting_point != NULL & msg.ending_point != NULL){
        paths_found->paths[0] = *compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.starting_point, msg.ending_point);
    }
    else if (msg.starting_point != NULL & msg.ending_point == NULL){
#pragma omp parallel for
        for (int i = 0; i < MAX_EXIT_POINTS; i++){
            if (msg.exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.exit_points[i], msg.ending_point);
                paths_found->paths[i] = *new_path;}
        }
    }
    else if (msg.starting_point == NULL & msg.ending_point != NULL){
# pragma omp parallel for
        for (int i = 0; i < MAX_EXIT_POINTS; i++){
            if (msg.exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.exit_points[i], msg.ending_point);
                paths_found->paths[i] = *new_path;
            }
        }
    }
    else if (msg.starting_point == NULL & msg.ending_point == NULL){
        int p = 0;
        // omp_lock_t lock;
        // omp_init_lock(&lock);
# pragma omp parallel for private(i,j) shared(p, msg, paths_found, MAX_EXIT_POINTS)
        for (int i = 0; i < MAX_EXIT_POINTS - 1; i++){
            for (int j = i + 1; j < MAX_EXIT_POINTS; j++){
                if (msg.exit_points[i] != NULL && msg.exit_points[i] != NULL && i != j){
                    ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.exit_points[i], msg.ending_point);
#pragma omp critical
                    {
                        paths_found->paths[p] = *new_path;
                    }
#pragma omp atomic
                    p++;
                }
            }
        }
        // omp_destroy_lock(&lock);
    }

    return paths_found;
}

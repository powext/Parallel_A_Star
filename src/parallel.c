#include <string.h>
#include "mpi.h"
#include "omp.h"
#include "../include/generic_list.h"
#include "../include/priority_queue.h"
#include "../include/print.h"
#include "../include/compute_path.h"

#define MAX_EXIT_POINTS 9

extern int HEIGHT;
extern int WIDTH;

MPI_Datatype create_node_datatype() {
    int blockcounts[7] = {1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype types[7] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[7];

    offsets[0] = offsetof(Node, id);
    offsets[1] = offsetof(Node, score);
    offsets[2] = offsetof(Node, distance);
    offsets[3] = offsetof(Node, heuristic);
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

void print_paths(MsgChunkEnd* paths){
    printf("Printing paths!\n");

    for (int i = 0; i < paths->num_of_paths; i++){
        printf("[(%d %d) -> (%d %d) = %d] => ", paths->paths[i].exit_points[0].x, paths->paths[i].exit_points[0].y, paths->paths[i].exit_points[1].x, paths->paths[i].exit_points[1].y, paths->paths[i].n_nodes);
        for(int j = 0; j < paths->paths[i].n_nodes; j++){
            printf("(%d %d)\t", paths->paths[i].nodes[j]->x, paths->paths[i].nodes[j]->y);
        }
        printf("\n----------\n");
    }
}


/* Compute the path using parallelism if it is needed
 * If both start and end point are in the same chunk the path will be one,
 * If only one is present, compute in parallel the paths from that and each exit points
 * If neither is present, compute the paths between each pair of exit points
 */
MsgChunkEnd* parallel_compute_paths(MsgChunkStart* msg){
    MsgChunkEnd* paths_found = malloc(sizeof(MsgChunkEnd));
    paths_found->num_of_paths = 0;
    paths_found->num_of_valid_paths = 0;

    // PATHS = (from one exit point to all the others) for each exit points
    //         + from the starting point to each exit points (if starting point is present)
    //         + from each exit points to the destination point (if destination point is present)
    //         + from the starting point to the destination point (if both are in the chunk)
    int total_points_num = msg->num_exit_points;
    int max_paths_num = msg->num_exit_points*(msg->num_exit_points-1); //MAX_EXIT_POINTS*(MAX_EXIT_POINTS-1);
    if (msg->starting_point != NULL){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (msg->ending_point != NULL){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (msg->starting_point != NULL && msg->ending_point != NULL){
        max_paths_num += 1;
    }

    paths_found->paths = malloc(max_paths_num*sizeof(ChunkPath));
    for (int i = 0; i < max_paths_num; i++){
        paths_found->paths[i].nodes = malloc(msg->chunk_w*msg->chunk_h*sizeof (Coordinates));
        paths_found->paths[i].exit_points = malloc(2*sizeof (Coordinates));
        paths_found->paths[i].n_nodes = 0;
    }

    Coordinates* total_points = malloc(total_points_num*sizeof(Coordinates));
    int index = 0;
    int starting_present = 0;
    if (msg->starting_point != NULL){
        total_points[index] = *msg->starting_point;
        index++;
        starting_present++;
    }
    for (index; index < msg->num_exit_points+starting_present; index++){
        total_points[index] = msg->exit_points[index-starting_present];
    }
    if (msg->ending_point != NULL){
        total_points[index] = *msg->ending_point;
    }

    for(int i = 0; i < total_points_num; i++){
        printf("Exit point %d: (%d %d)\n", i, total_points[i].x, total_points[i].y);
    }

    printf("Finding path between exit points\n");
    int p = 0;
#pragma omp parallel for shared(p, msg, paths_found, total_points_num, total_points) default(none) collapse(2) num_threads(total_points_num)
    for (int i = 0; i < total_points_num; i++){
        for (int j = i + 1; j < total_points_num; j++){
            if (&total_points[i] != NULL && &total_points[j] != NULL && i != j) {
                ChunkPath *new_path = compute_path(msg->matrix, msg->chunk_w, msg->chunk_h, total_points[i],
                                                   total_points[j]);
#pragma omp critical
                {
                    paths_found->num_of_paths += 1;
                    paths_found->paths[p] = *new_path;
                    if (new_path->n_nodes > 0)
                        paths_found->num_of_valid_paths += 1;
                    p++;
                }
            }
        }
    }

    print_paths(paths_found);
    free(total_points);
    return paths_found;
}


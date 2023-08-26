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

void print_paths(MsgChunkEnd* paths){
    printf("Printing paths!\n");

    for (int i = 0; i < paths->num_of_paths; i++){
        for(int j = 0; j < paths->paths[i].n_nodes; j++){
            printf("(%d %d)\t", paths->paths[i].nodes[j]->x, paths->paths[i].nodes[j]->y);
        }
        printf("\n----------\n");
    }
}

void swap(ChunkPath *a, ChunkPath *b) {
    ChunkPath temp = *a;
    *a = *b;
    *b = temp;
}

void sort_paths_by_length(ChunkPath *pathsList, int num_paths, int num_valid_paths) {
    if(num_valid_paths < 1){
        return;
    }
    for (int i = 0; i < num_paths - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < num_paths - i - 1; j++) {
            if ((pathsList[j].n_nodes == 0 && pathsList[j + 1].n_nodes != 0) ||
                (pathsList[j].n_nodes > pathsList[j + 1].n_nodes && pathsList[j + 1].n_nodes != 0)) {
                swap(&pathsList[j], &pathsList[j + 1]);
                swapped = true;
            }
        }

        // If no two elements were swapped by inner loop, the array is already sorted
        if (!swapped) {
            break;
        }
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
    int max_paths_num = MAX_EXIT_POINTS*(MAX_EXIT_POINTS-1);
    paths_found->paths = malloc(max_paths_num*sizeof(ChunkPath));
    for (int i = 0; i < max_paths_num; i++){
        paths_found->paths[i].nodes = malloc(msg->chunk_w*msg->chunk_h*sizeof (Coordinates));
        paths_found->paths[i].exit_points = malloc(2*sizeof (Coordinates));
        paths_found->paths[i].n_nodes = 0;
    }

    if (msg->starting_point != NULL && msg->ending_point != NULL){
        printf("Finding path between start and end\n");
        ChunkPath* new_path = compute_path(msg->matrix, msg->chunk_w, msg->chunk_h, *msg->starting_point, *msg->ending_point);
        paths_found->num_of_paths = 1;
        if (new_path->n_nodes > 0){
            paths_found->paths = new_path;
            paths_found->num_of_valid_paths = 1;
        }
    }

    else if (msg->starting_point != NULL && msg->ending_point == NULL){
        printf("Finding path between start and exit points\n");
#pragma omp parallel for shared(msg, paths_found) default(none)
        for (int i = 0; i < msg->num_exit_points; i++){
            if (&msg->exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg->matrix, msg->chunk_w, msg->chunk_h, *msg->starting_point, msg->exit_points[i]);
#pragma omp atomic
                paths_found->num_of_paths += 1;

                if (new_path->n_nodes > 0){
                    paths_found->paths[i] = *new_path;
#pragma omp atomic
                    paths_found->num_of_valid_paths += 1;
                }
            }
        }
    }

    else if (msg->starting_point == NULL && msg->ending_point != NULL){
        printf("Finding path between exit points and end\n");
# pragma omp parallel for default(none) shared(msg, paths_found)
        for (int i = 0; i < msg->num_exit_points; i++){
            if (&msg->exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg->matrix, msg->chunk_w, msg->chunk_h, msg->exit_points[i], *msg->ending_point);

#pragma omp atomic
                paths_found->num_of_paths += 1;
                if (new_path->n_nodes > 0){
                    paths_found->paths[i] = *new_path;
#pragma omp atomic
                    paths_found->num_of_valid_paths += 1;
                }
            }
        }
    }

    else if (msg->starting_point == NULL && msg->ending_point == NULL){
        printf("Finding path between exit points\n");
        int p = 0;
# pragma omp parallel for shared(p, msg, paths_found) default(none)
        for (int i = 0; i < msg->num_exit_points - 1; i++){
            for (int j = i + 1; j < msg->num_exit_points; j++){
                if (&msg->exit_points[i] != NULL && &msg->exit_points[j] != NULL && i != j) {
                    ChunkPath *new_path = compute_path(msg->matrix, msg->chunk_w, msg->chunk_h, msg->exit_points[i],
                                                       msg->exit_points[j]);
#pragma omp atomic
                    paths_found->num_of_paths += 1;

                    if (new_path->n_nodes > 0) {
#pragma omp critical
                        {
                            paths_found->paths[p] = *new_path;
                            p++;
                            paths_found->num_of_valid_paths += 1;
                        }
                    }
                }
            }
        }
    }

    sort_paths_by_length(paths_found->paths, paths_found->num_of_paths, paths_found->num_of_valid_paths);
    print_paths(paths_found);
    return paths_found;
}


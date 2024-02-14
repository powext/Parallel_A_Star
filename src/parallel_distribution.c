#include "../include/parallel_distribution.h"

#include <string.h>

#include "../include/exit_points.h"
#include "../include/cJSON.h"
#include "../include/utility.h"
#include "../include/parallel_collection.h"
#include "../include/compute_distance.h"
#include "../include/parallel_paths.h"
#include "time.h"

extern bool DEBUG;
extern int N_EXIT_POINTS_PER_CHUNK;

MPI_Datatype create_node_datatype() {
    MPI_Datatype CoordType;
    MPI_Type_contiguous(2, MPI_INT, &CoordType);
    MPI_Type_commit(&CoordType);

    int blockcounts[6] = {1, 1, 1, 1, 1, 1};
    MPI_Datatype types[6] = {MPI_INT, MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE, CoordType, MPI_INT};
    MPI_Aint offsets[6];

    offsets[0] = offsetof(Node, id);
    offsets[1] = offsetof(Node, score);
    offsets[2] = offsetof(Node, distance);
    offsets[3] = offsetof(Node, heuristic);
    offsets[4] = offsetof(Node, coordinates);
    offsets[5] = offsetof(Node, type);

    MPI_Datatype mpi_node_type;
    MPI_Type_create_struct(6, blockcounts, offsets, types, &mpi_node_type);
    MPI_Type_commit(&mpi_node_type);

    return mpi_node_type;
}

MPI_Datatype pack_MsgStart(MsgChunkStart *msg, char** buffer, int* size) {
    *size = sizeof(MsgChunkStart) + msg->num_exit_points * sizeof(Coordinates);
    *buffer = (char *)malloc(*size);
    memcpy(*buffer, msg, sizeof(MsgChunkStart));
    memcpy(*buffer + sizeof(MsgChunkStart), msg->exit_points, msg->num_exit_points * sizeof(Coordinates));
}

void unpackMsgChunkStart(char *buffer, MsgChunkStart **msg) {
    *msg = malloc(sizeof(MsgChunkStart));
    memcpy(*msg, buffer, sizeof(MsgChunkStart));
    (*msg)->exit_points = (Coordinates *)malloc((*msg)->num_exit_points * sizeof(Coordinates));
    memcpy((*msg)->exit_points, buffer + sizeof(MsgChunkStart), (*msg)->num_exit_points * sizeof(Coordinates));
}

bool is_point_contained(Node* l_nodes, int chunk_side_length, Coordinates* point) {
    Coordinates initial = l_nodes[0].coordinates;
    if (point->x < initial.x || point->x >= initial.x + chunk_side_length || point->y < initial.y || point->y >= initial.y + chunk_side_length){
        return false;
    }
    return true;
}

MsgChunkEnd* distribute_work(Node *nodes, AdjList** graph, MsgChunkStart** start_msgs, int size, Node *starting_node, Node *destination_node, int n_chunks, int world_rank, clock_t *step_start_time) {
    int chunk_side_length, chunk_size, n_chunks_per_side;
    int *send_count;
    int *displacements;
    MPI_Datatype node_type = create_node_datatype();
    MPI_Datatype node_vector_type, node_vector_type_resized;
    if (world_rank == 0) {
        if(DEBUG)
            printf_debug("Matrix dimension: %d\n", size);

        if (sqrt(n_chunks) != (int) sqrt(n_chunks)) {
            printf_debug("The number of processes must be a perfect square.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (size * size % n_chunks != 0) {
            printf_debug("The total size of the array must be a multiple of the number of processes.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        n_chunks_per_side = (int) sqrt(n_chunks);
        chunk_size = size * size;
        chunk_side_length = (int) sqrt((double) chunk_size / n_chunks);
        if (chunk_side_length * chunk_side_length * n_chunks != size * size) {
            printf_debug("The total size of the array must be a perfect square multiple of the number of processes.\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (DEBUG) {
            printf_debug("Chunk_side_length: %d\n", chunk_side_length);
            printf_debug("N_chunks: %d\n", n_chunks);
        }

        displacements = malloc(sizeof(int) * n_chunks);
        for (int j = 0; j < n_chunks_per_side; j++) {
            for (int i = 0; i < n_chunks_per_side; i++) {
                displacements[j * n_chunks_per_side + i] = (j * n_chunks_per_side * chunk_side_length) + i;
            }
        }

        if (DEBUG) {
            printf_debug("Displacements: ");
            for (int i = 0; i < n_chunks; i++) {
                if (!DEBUG) continue;
                printf("%d ", displacements[i]);
            }
            printf_debug("\n");
        }

        send_count = malloc(sizeof(int) * n_chunks);
        for (int i = 0; i < n_chunks; i++)
            send_count[i] = 1;

        MPI_Type_vector(chunk_side_length, chunk_side_length, size, node_type, &node_vector_type);
        MPI_Type_commit(&node_vector_type);
        MPI_Type_create_resized(node_vector_type, 0, (MPI_Aint) (chunk_side_length * sizeof(Node)),
                                &node_vector_type_resized);
        MPI_Type_commit(&node_vector_type_resized);
    }

    if (world_rank == 0) {
        for (int i = 1; i < n_chunks; i++) {
            MPI_Send(&chunk_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    } else {
        MPI_Recv(&chunk_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        chunk_side_length = (int) sqrt((double) chunk_size / n_chunks);
    }

    Node* l_nodes = malloc((chunk_side_length * chunk_side_length) * sizeof(Node));
    if (MPI_Scatterv(
            world_rank == 0 ? nodes : NULL,
            world_rank == 0 ? send_count : NULL,
            world_rank == 0 ? displacements : NULL,
            world_rank == 0 ? node_vector_type_resized : NULL,
            l_nodes,
            chunk_size,
            node_type,
            0,
            MPI_COMM_WORLD)) {
        printf_debug("Scatter error\n");
        exit(1);
    }

    printf_debug("Creating MsgStart Datatype!\n");

    printf_debug("Allocating MsgStart!\n");
    *start_msgs = malloc((sizeof(MsgChunkStart) + N_EXIT_POINTS_PER_CHUNK * sizeof(Coordinates)) * n_chunks);
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 0) {
#pragma omp parallel for shared(chunk_side_length, nodes)
        for (int i = 0; i < n_chunks; i++) {
            printf_debug("Creating msg %d\n", i);
            MsgChunkStart *local_msg =  malloc(sizeof(MsgChunkStart) + N_EXIT_POINTS_PER_CHUNK * sizeof(Coordinates));

            local_msg->chunk_w = chunk_side_length;
            local_msg->chunk_h = chunk_side_length;
            local_msg->starting_point = starting_node->coordinates;
            local_msg->ending_point = destination_node->coordinates;
            local_msg->num_exit_points = N_EXIT_POINTS_PER_CHUNK;

            local_msg->exit_points = (Coordinates *)malloc( N_EXIT_POINTS_PER_CHUNK * sizeof(Coordinates));
            for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                local_msg->exit_points[j].x = NULL_COORD;
                local_msg->exit_points[j].y = NULL_COORD;
            }

            Coordinates initial = {
                    .x = (displacements[i] % size) * chunk_side_length,
                    .y = (displacements[i] / size) * chunk_side_length
            };
#pragma omp critical
            {
                find_chunk_corners_exit_points(world_rank, nodes, size, chunk_side_length,
                                                                            initial, local_msg->exit_points);
                find_chunk_sides_exit_points(world_rank, nodes, size, chunk_side_length,
                                                                          initial, local_msg->exit_points);
                local_msg->num_exit_points = N_EXIT_POINTS_PER_CHUNK;

                if (DEBUG) {
                    printf_debug("[THREAD %d] Exit_points: ", omp_get_thread_num());
                    for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                        if (!DEBUG) continue;
                        printf("%d:%d ", local_msg->exit_points[j].x, local_msg->exit_points[j].y);
                    }
                    printf_debug("\n");
                }
            }

            (*start_msgs)[i] = *local_msg;
        }
        MPI_Type_free(&node_type);
        MPI_Type_free(&node_vector_type);
        MPI_Type_free(&node_vector_type_resized);
        free(send_count);
    }

    MsgChunkStart* msg;
    printf_debug("sizeof(MsgChunkStart): %zu\n", sizeof(MsgChunkStart));
    printf_debug("sizeof(Coordinates): %zu\n", sizeof(Coordinates));
    printf_debug("N_EXIT_POINTS_PER_CHUNK: %d\n", N_EXIT_POINTS_PER_CHUNK);


    char *packedBuffer;
    int bufferSize;
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        for (int i = 1; i < n_chunks; ++i) {
            printf_debug("Sending Message %d\n", i);
            printf_debug("Size of data being sent: %zu\n", sizeof((*start_msgs)[i]));

            pack_MsgStart(&(*start_msgs)[i], &packedBuffer, &bufferSize);
            if (MPI_Send(packedBuffer, bufferSize, MPI_PACKED, i, 0, MPI_COMM_WORLD)) {
                printf_debug("Send error\n");
                exit(1);
            }
        }
        printf_debug("Assigning Message 0\n");
        msg = malloc(sizeof(MsgChunkStart) + N_EXIT_POINTS_PER_CHUNK * sizeof(Coordinates));
        msg = &(*start_msgs)[0];
        if (!is_point_contained(l_nodes, chunk_side_length, &msg->starting_point)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        } if (!is_point_contained(l_nodes,chunk_side_length, &msg->ending_point)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
    } else {
        printf_debug("Receiving Messages\n");

        if (MPI_Recv(packedBuffer, bufferSize, MPI_PACKED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) {
            printf_debug("Recv error\n");
            exit(1);
        }
        unpackMsgChunkStart(packedBuffer, &msg);

        if (!is_point_contained(l_nodes, chunk_side_length, &msg->starting_point)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        } if (!is_point_contained(l_nodes, chunk_side_length, &msg->ending_point)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
        printf_debug("Received Messages\n");
        printf_debug("Start: (%d:%d)\n", msg->starting_point.x, msg->starting_point.y);
        printf_debug("End: (%d:%d)\n", msg->ending_point.x, msg->ending_point.y);
    }

    // debug msg, one rank at the time
    for (int i = 0; i < n_chunks; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (world_rank == i) {
            printf("Msg: \n");
            printf("Chunk_w: %d\n", msg->chunk_w);
            printf("Chunk_h: %d\n", msg->chunk_h);
            printf("Starting point: %d:%d\n", msg->starting_point.x, msg->starting_point.y);
            printf("Ending point: %d:%d\n", msg->ending_point.x, msg->ending_point.y);
            printf("Exit points: \n");
            for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++){
                printf("(%d %d) ", msg->exit_points[j].x, msg->exit_points[j].y);
            }
        }
    }

    if (world_rank == 0) {
        clock_t step_time_end = clock();
        double diff_time_final_computation = (double)(step_time_end - *step_start_time) / CLOCKS_PER_SEC;
        printf("Distribution took: %2f seconds\n", diff_time_final_computation);
        *step_start_time = clock();
    }

    MsgChunkEnd* msgChunkEnd = parallel_compute_paths(msg, l_nodes, size, world_rank);

    if (world_rank == 0) {
        *graph = create_graph(size*size);
    }

    MsgChunkEnd *receivedMsgs = collect_msgs_end(msgChunkEnd, nodes, size, world_rank, n_chunks, *graph);

    if (world_rank == 0) {
        clock_t step_time_end = clock();
        double diff_time_final_computation = (double)(step_time_end - *step_start_time) / CLOCKS_PER_SEC;
        printf("Chunks computation took: %2f seconds\n", diff_time_final_computation);
        *step_start_time = clock();
    }

    free(l_nodes);
    if (world_rank == 0) {
        free(displacements);
        free(packedBuffer);
    }
    return receivedMsgs;
}

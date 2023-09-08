#include "../include/parallel_distribution.h"
#include "../include/exit_points.h"
#include "../include/cJSON.h"
#include "../include/utility.h"
#include "../include/json_output.h"
#include "../include/parallel_paths.h"
#include "../include/parallel_collection.h"

extern int DEBUG;
extern int DEBUG_PROCESS;

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

MPI_Datatype create_MsgStart_datatype() {
    MPI_Datatype mpi_coordinates;
    int blocklengths_coords[2] = {1, 1};
    MPI_Datatype types_coords[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets_coords[2];

    offsets_coords[0] = offsetof(Coordinates, x);
    offsets_coords[1] = offsetof(Coordinates, y);

    MPI_Type_create_struct(2, blocklengths_coords, offsets_coords, types_coords, &mpi_coordinates);
    MPI_Type_commit(&mpi_coordinates);

    MPI_Datatype mpi_msgchunkstart;
    int blocklengths_chunk[6] = {1, 1, 1, 1, 1, N_EXIT_POINTS_PER_CHUNK};
    MPI_Datatype types_chunk[6] = {MPI_INT, MPI_INT, mpi_coordinates, mpi_coordinates, MPI_INT,mpi_coordinates};
    MPI_Aint offsets_chunk[6];

    offsets_chunk[0] = offsetof(MsgChunkStart, chunk_w);
    offsets_chunk[1] = offsetof(MsgChunkStart, chunk_h);
    offsets_chunk[2] = offsetof(MsgChunkStart, starting_point);
    offsets_chunk[3] = offsetof(MsgChunkStart, ending_point);
    offsets_chunk[4] = offsetof(MsgChunkStart, num_exit_points);
    offsets_chunk[5] = offsetof(MsgChunkStart, exit_points);

    MPI_Type_create_struct(6, blocklengths_chunk, offsets_chunk, types_chunk, &mpi_msgchunkstart);
    MPI_Type_commit(&mpi_msgchunkstart);
    return mpi_msgchunkstart;

}

MPI_Datatype create_ChunkPath_datatype(int size){
    MPI_Datatype mpi_coordinates;
    int blocklengths_coords[2] = {1, 1};
    MPI_Datatype types_coords[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets_coords[2];

    offsets_coords[0] = offsetof(Coordinates, x);
    offsets_coords[1] = offsetof(Coordinates, y);

    MPI_Type_create_struct(2, blocklengths_coords, offsets_coords, types_coords, &mpi_coordinates);
    MPI_Type_commit(&mpi_coordinates);

}

MPI_Datatype create_MsgEnd_datatype(int size){
    MPI_Datatype mpi_coordinates;
    int blocklengths_coords[2] = {1, 1};
    MPI_Datatype types_coords[2] = {MPI_INT, MPI_INT};
    MPI_Aint offsets_coords[2];

    offsets_coords[0] = offsetof(Coordinates, x);
    offsets_coords[1] = offsetof(Coordinates, y);

    MPI_Type_create_struct(2, blocklengths_coords, offsets_coords, types_coords, &mpi_coordinates);
    MPI_Type_commit(&mpi_coordinates);

    MPI_Datatype mpi_chunkpath;
    int chunk_size = size*size;

    int blocklengths_chunks[3] = {1, 2, chunk_size};
    MPI_Datatype types_chunks[3] = {MPI_INT, mpi_coordinates, mpi_coordinates};
    MPI_Aint offsets_chunks[3];

    offsets_chunks[0] = offsetof(ChunkPath, n_nodes);
    offsets_chunks[1] = offsetof(ChunkPath, exit_points);
    offsets_chunks[2] = offsetof(ChunkPath, nodes);

    MPI_Type_create_struct(3, blocklengths_chunks, offsets_chunks, types_chunks, &mpi_chunkpath);
    MPI_Type_commit(&mpi_chunkpath);

    //MPI_Datatype mpi_chunkpath = create_ChunkPath_datatype(size);

    MPI_Datatype mpi_msgchunkend;
    int max_paths = (N_EXIT_POINTS_PER_CHUNK+2)*(N_EXIT_POINTS_PER_CHUNK+1);
    int blocklengths_chunk[3] = {1, 1, max_paths};
    MPI_Datatype types_chunk[3] = {MPI_INT, MPI_INT, mpi_chunkpath};
    MPI_Aint offsets_chunk[3];

    offsets_chunk[0] = offsetof(MsgChunkEnd, num_of_paths);
    offsets_chunk[1] = offsetof(MsgChunkEnd, num_of_valid_paths);
    offsets_chunk[2] = offsetof(MsgChunkEnd, paths);

    MPI_Type_create_struct(3, blocklengths_chunk, offsets_chunk, types_chunk, &mpi_msgchunkend);
    MPI_Type_commit(&mpi_msgchunkend);
    return mpi_msgchunkend;
}

bool is_point_contained(Node* l_nodes, int chunk_side_length, Coordinates* point){
    Coordinates initial = l_nodes[0].coordinates;
    if (point->x < initial.x || point->x >= initial.x + chunk_side_length || point->y < initial.y || point->y >= initial.y + chunk_side_length){
        return false;
    }
    return true;
}

// TODO: return complete path as ChunkPath where exit points are starting and ending points?
void distribute_work(Node *nodes, int size, Node *starting_node, Node *destination_node, int n_chunks, int world_rank) {
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
                if (DEBUG_PROCESS > 0 && DEBUG_PROCESS != world_rank) continue;
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
    MPI_Datatype msgStartDatatype = create_MsgStart_datatype();

    printf_debug("Allocating MsgStart!\n");
    MsgChunkStart *messages = malloc(sizeof(MsgChunkStart) * n_chunks);
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 0) {
#pragma omp parallel for shared(chunk_side_length, nodes)
        for (int i = 0; i < n_chunks; i++) {

            printf_debug("Creating msg %d\n", i);
            MsgChunkStart local_msg = {
                    .chunk_w = chunk_side_length,
                    .chunk_h = chunk_side_length,
                    .starting_point = starting_node->coordinates,
                    .ending_point = destination_node->coordinates,
                    .num_exit_points = 0,
                    .exit_points = {
                            [0 ... N_EXIT_POINTS_PER_CHUNK-1] = {NULL_COORD, NULL_COORD}
                    }
            };

            Coordinates initial = {
                    .x = (displacements[i] % size) * chunk_side_length,
                    .y = (displacements[i] / size) * chunk_side_length
            };
#pragma omp critical
            {
                local_msg.num_exit_points += find_chunk_corners_exit_points(world_rank, nodes, size, chunk_side_length,
                                                                            initial, local_msg.exit_points);
                local_msg.num_exit_points += find_chunk_sides_exit_points(world_rank, nodes, size, chunk_side_length,
                                                                          initial, local_msg.exit_points);

                printf_debug("[THREAD %d] Number of exit points: %d\n",
                           omp_get_thread_num(), local_msg.num_exit_points);

                if (DEBUG) {
                    printf_debug("[THREAD %d] Exit_points: ", omp_get_thread_num());
                    for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                        printf_debug("%d:%d ", local_msg.exit_points[j].x, local_msg.exit_points[j].y);
                    }
                    printf_debug("\n");
                }
            }

            messages[i] = local_msg;
        }
        MPI_Type_free(&node_type);
        MPI_Type_free(&node_vector_type);
        MPI_Type_free(&node_vector_type_resized);
        free(send_count);

    }

    MsgChunkStart* msg = malloc(sizeof(MsgChunkStart));

    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        for (int i = 1; i < n_chunks; ++i) {
            if (DEBUG)
                printf_debug("Sending Message %d\n", i);
            if (MPI_Send(&messages[i], 1, msgStartDatatype, i, 0, MPI_COMM_WORLD)) {
                printf_debug("[Send error\n");
                exit(1);
            }
        }
        if (DEBUG)
            printf_debug("Assigning Message 0\n");
        msg = &messages[0];
        if (!is_point_contained(l_nodes, chunk_side_length, &msg->starting_point)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        } if (!is_point_contained(l_nodes,chunk_side_length, &msg->ending_point)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
    } else {
        if (DEBUG)
            printf_debug("Receiving Messages\n");

        if (MPI_Recv(msg, 1, msgStartDatatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) {
            printf_debug("Recv error\n");
            exit(1);
        }
        if (!is_point_contained(l_nodes, chunk_side_length, &msg->starting_point)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        } if (!is_point_contained(l_nodes, chunk_side_length, &msg->ending_point)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
        if (DEBUG) {
            printf_debug("Received Messages\n");
            printf_debug("Start: (%d:%d)\n", msg->starting_point.x, msg->starting_point.y);
            printf_debug("End: (%d:%d)\n", msg->ending_point.x, msg->ending_point.y);
        }
    }

    MsgChunkEnd* computed_paths = parallel_compute_paths(msg, l_nodes, size, world_rank);
    printf_debug("Found %d paths/%d valid\n", computed_paths->num_of_paths, computed_paths->num_of_valid_paths);
    /// TODO: Implement chunks computing
    MsgChunkEnd* msgChunkEnd = get_dummy_endmsg(world_rank);

    MsgChunkEnd *receivedMsgs = collect_msgs_end(msgChunkEnd, world_rank, n_chunks);

    output_json(nodes, starting_node, destination_node, messages, world_rank, n_chunks);

    if (world_rank == 0) {
        for (int i = 0; i < n_chunks; i++) {
            for (int j = 0; j < receivedMsgs[i].num_of_paths; j++) {
                free(receivedMsgs[i].paths[j].nodes);
                free(receivedMsgs[i].paths[j].exit_points);
            }
            free(receivedMsgs[i].paths);
        }
    }
    free(receivedMsgs);
    free(l_nodes);
    MPI_Type_free(&msgStartDatatype);
    MPI_Type_free(&msgEndDatatype);
}

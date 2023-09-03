#import "../include/parallel_distribution.h"
#include "../include/exit_points.h"
#include "../include/cJSON.h"
#include "../include/util.h"
#include "../include/json_output.h"

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
    int blocklengths_chunk[5] = {1, 1, 1, 1, N_EXIT_POINTS_PER_CHUNK};
    MPI_Datatype types_chunk[5] = {MPI_INT, MPI_INT, mpi_coordinates, mpi_coordinates, mpi_coordinates};
    MPI_Aint offsets_chunk[5];

    offsets_chunk[0] = offsetof(MsgChunkStart, chunk_w);
    offsets_chunk[1] = offsetof(MsgChunkStart, chunk_h);
    offsets_chunk[2] = offsetof(MsgChunkStart, starting_point);
    offsets_chunk[3] = offsetof(MsgChunkStart, ending_point);
    offsets_chunk[4] = offsetof(MsgChunkStart, exit_points);

    MPI_Type_create_struct(5, blocklengths_chunk, offsets_chunk, types_chunk, &mpi_msgchunkstart);
    MPI_Type_commit(&mpi_msgchunkstart);
    return mpi_msgchunkstart;
}

void distribute_work(Node *nodes, Node *starting_node, Node *destination_node, int n_chunks, int world_rank) {
    int chunk_side_length, chunk_size, n_chunks_per_side;
    int *send_count;
    int *displacements;
    MPI_Datatype node_type = create_node_datatype();
    MPI_Datatype node_vector_type, node_vector_type_resized;
    if (world_rank == 0) {
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

        n_chunks_per_side = (int) sqrt(n_chunks);
        chunk_size = GRID_WIDTH * GRID_HEIGHT;
        chunk_side_length = (int) sqrt((double) chunk_size / n_chunks);
        if (chunk_side_length * chunk_side_length * n_chunks != GRID_WIDTH * GRID_HEIGHT) {
            if (world_rank == 0) {
                printf("[ERROR] The total size of the array must be a perfect square multiple of the number of processes.\n");
            }
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (DEBUG && world_rank == 0) {
            printf("[DEBUG] R0 - chunk_side_length: %d\n", chunk_side_length);
            printf("[DEBUG] R0 - n_chunks: %d\n", n_chunks);
        }

        displacements = malloc(sizeof(int) * n_chunks);
        for (int j = 0; j < n_chunks_per_side; j++) {
            for (int i = 0; i < n_chunks_per_side; i++) {
                displacements[j * n_chunks_per_side + i] = (j * n_chunks_per_side * chunk_side_length) + i;
            }
        }

        if (DEBUG && world_rank == 0) {
            printf("[DEBUG] R0 - Displacements: ");
            for (int i = 0; i < n_chunks; i++) {
                printf("%d ", displacements[i]);
            }
            printf("\n");
        }

        send_count = malloc(sizeof(int) * n_chunks);
        for (int i = 0; i < n_chunks; i++)
            send_count[i] = 1;

        MPI_Type_vector(chunk_side_length, chunk_side_length, GRID_WIDTH, node_type, &node_vector_type);
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

    Node *l_nodes = malloc(sizeof(Node) * chunk_side_length * chunk_side_length);
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
        printf("Error: Scatter error\n");
        exit(1);
    }
    printf("[DEBUG] Received chunk %d\n", world_rank);

    MPI_Datatype msgStartDatatype = create_MsgStart_datatype();
    MsgChunkStart *messages = malloc(sizeof(MsgChunkStart) * n_chunks);
    // debug(0);
    if (world_rank == 0) {
#pragma omp parallel for

        for (int i = 0; i < n_chunks; i++) {
            MsgChunkStart local_msg = {
                    .chunk_w = chunk_side_length,
                    .chunk_h = chunk_side_length,
                    .starting_point = {starting_node->coordinates.x, starting_node->coordinates.y},
                    .ending_point = {destination_node->coordinates.x, destination_node->coordinates.y},
                    .exit_points = {
                            [0 ... N_EXIT_POINTS_PER_CHUNK-1] = {NULL_COORD, NULL_COORD}
                    }
            };

            Coordinates initial = {
                    .x = (displacements[i] % GRID_WIDTH) * chunk_side_length,
                    .y = (displacements[i] / GRID_WIDTH) * chunk_side_length
            };
            find_chunk_corners_exit_points(nodes, chunk_side_length, initial, local_msg.exit_points);
            find_chunk_sides_exit_points(nodes, chunk_side_length, initial, local_msg.exit_points);
            if (DEBUG) {
                printf("[DEBUG] R0 - exit_points: ");
                for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                    printf("%d:%d ", local_msg.exit_points[j].x, local_msg.exit_points[j].y);
                }
                printf("\n");
            }

            messages[i] = local_msg;
            if (MPI_Send(&local_msg, 1, msgStartDatatype, i, 0, MPI_COMM_WORLD)) {
                printf("[Error]: R0 - Send error\n");
                exit(1);
            }

        }
        MPI_Type_free(&node_type);
        MPI_Type_free(&node_vector_type);
        MPI_Type_free(&node_vector_type_resized);
        free(displacements);
        free(send_count);
    }
    MsgChunkStart msg;
    if (MPI_Recv(&msg, 1, msgStartDatatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) {
        printf("[Error]: Recv error\n");
        exit(1);
    }
    printf("[DEBUG] Received MsgStart %d\n", world_rank);

    /// TODO: Implement chunks computing

    output_json(nodes, starting_node, destination_node, messages, world_rank, n_chunks);

    free(l_nodes);
    MPI_Type_free(&msgStartDatatype);
}

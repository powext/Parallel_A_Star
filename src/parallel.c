#include <printf.h>
#include <math.h>
#include <tic.h>
#include <unistd.h>
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

void find_chunk_corners_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    enum Direction {
        up, down, left, right
    };
    struct Angle {
        Coordinates coordinates;
        enum Direction directions[2];
    };
    struct Angle corners[4] = {
            {initial, {up, left}},
            {initial.x + chunk_side_length -1, initial.y, {right, up}},
            {initial.x + chunk_side_length -1, initial.y + chunk_side_length -1, {down, right}},
            {initial.x, initial.y + chunk_side_length -1, {left, down}}
    };
    for (int i = 0; i < 4; i++) {
        Coordinates corner = corners[i].coordinates;
        enum Direction* directions = corners[i].directions;
        if (DEBUG) {
            printf("[DEBUG] Corner: %d:%d\n", corner.x, corner.y);
        }
        for (int j = 0; j < 2; j++) {
            enum Direction direction = directions[j];
            if (DEBUG) {
                printf("[DEBUG] Direction: %d\n", direction);
            }
            Coordinates outlet_coords = corner;
            switch (direction) {
                case up:
                    outlet_coords.y = corner.y - 1;
                    break;
                case down:
                    outlet_coords.y = corner.y + 1;
                    break;
                case left:
                    outlet_coords.x = corner.x - 1;
                    break;
                case right:
                    outlet_coords.x = corner.x + 1;
                    break;
            }
            if (DEBUG) {
                printf("[DEBUG] Exit point: %d:%d\n", outlet_coords.x, outlet_coords.y);
            }
            if (outlet_coords.x < 0 || outlet_coords.x >= GRID_WIDTH || outlet_coords.y < 0 || outlet_coords.y >= GRID_HEIGHT) {
                if (DEBUG) {
                    printf("[DEBUG] Exit point out of bounds\n");
                }
                continue;
            }
            Node* outlet_node = &nodes[outlet_coords.y * GRID_WIDTH + outlet_coords.x];
            if (outlet_node->type == obstacle) {
                if (DEBUG) {
                    printf("[DEBUG] Exit point is an obstacle\n");
                }
                continue;
            }
            Node* exit_point = &nodes[corner.y * GRID_WIDTH + corner.x];
            exit_points[i] = exit_point->coordinates;
            if (DEBUG) {
                printf("[DEBUG] Exit point is valid\n");
            }
        }
    }
}

void find_exit_points_on_vector(Node* nodes, Coordinates *vector, Coordinates* outer_vector, int vector_length, Coordinates *exit_points, int direction) {
    // debug vectors
    if (DEBUG) {
        printf("[DEBUG] R0 - vector: ");
        for (int i = 0; i < vector_length; i++) {
            printf("%d:%d ", vector[i].x, vector[i].y);
        }
        printf("\n");
        printf("[DEBUG] R0 - outer_vector: ");
        for (int i = 0; i < vector_length; i++) {
            printf("%d:%d ", outer_vector[i].x, outer_vector[i].y);
        }
        printf("\n");
    }
    int N = (N_EXIT_POINTS_PER_CHUNK - 4) / 4;
    int step = (vector_length - 2) / N; // Exclude the first and last entry

    for (int i = 1; i <= N; i++) {
        int index = 1 + (i * step) - (step / 2); // Start from the middle of the step, +1 to skip the first entry

        // Check the middle point first
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
            && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            printf("Writing in %d\n", 4 + (direction * N) + (i - 1));
            exit_points[4 + (direction * N) + (i - 1)] = vector[index];
            continue;
        }

        // Searching to the left from the middle
        int leftIndex = index - 1;
        while (leftIndex > 1 && nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type == obstacle) { // > 1 to avoid the first entry
            leftIndex--;
        }

        // If a valid point is found to the left, add to exit_points
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
            && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            printf("Writing in %d\n", 4 + (direction * N - 1) + i - 1);
            exit_points[4 + ((direction - 1) * (N - 1)) + i] = vector[leftIndex];
            continue;
        }

        // If not found in the middle or to the left, searching to the right
        int rightIndex = index + 1;
        while (rightIndex < vector_length - 1
            && (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type == obstacle
            || nodes[vector->y * GRID_WIDTH + vector->x].type == obstacle)) {
            rightIndex++;
        }
        // If a valid point is found to the right, add to exit_points
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
        && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            printf("Writing in %d\n", 4 + (direction * N - 1) + i - 1);
            exit_points[4 + ((direction - 1) * (N - 1)) + i] = vector[rightIndex];
        }
    }
}

void find_chunk_sides_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    Coordinates vector1a[chunk_side_length];
    Coordinates vector1b[chunk_side_length];
    Coordinates vector2a[chunk_side_length];
    Coordinates vector2b[chunk_side_length];
    // Debug
    if (DEBUG) {
        printf("[DEBUG] R0 - initial: %d:%d\n", initial.x, initial.y);
    }
    // Iterating top and bottom
    if ((initial.y - 1) >= 0) {
        printf("[DEBUG] R0 - ok top\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[initial.y * GRID_WIDTH + initial.x + i].coordinates;
            vector1b[i] = nodes[(initial.y - 1) * GRID_WIDTH + initial.x + i].coordinates;
        }
        find_exit_points_on_vector(nodes, vector1a, vector1b, chunk_side_length, exit_points, 0);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of top\n");
    }

    if ((initial.y + chunk_side_length) < GRID_HEIGHT) {
        printf("[DEBUG] R0 - ok bottom\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + chunk_side_length - 1) * GRID_WIDTH + initial.x + i].coordinates;
            vector2b[i] = nodes[(initial.y + chunk_side_length) * GRID_WIDTH + initial.x + i].coordinates;
        }
        find_exit_points_on_vector(nodes, vector2a, vector2b, chunk_side_length, exit_points, 1);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of bottom\n");
    }

    // Iterating left and right
    // Debug
    if ((initial.x - 1) >= 0) {
        printf("[DEBUG] R0 - ok left\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x].coordinates;
            vector1b[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x - 1].coordinates;
        }
        find_exit_points_on_vector(nodes, vector1a, vector1b, chunk_side_length, exit_points, 2);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of left\n");
    }
    if ((initial.x + chunk_side_length) < GRID_WIDTH) {
        printf("[DEBUG] R0 - ok right\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x + chunk_side_length - 1].coordinates;
            vector2b[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x + chunk_side_length].coordinates;
        }
        find_exit_points_on_vector(nodes, vector2a, vector2b, chunk_side_length, exit_points, 3);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of right\n");
    }
}

void distribute_work(Node* nodes, Node* starting_node, Node* destination_node, int n_chunks, int world_rank) {
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

    int n_chunks_per_side = (int) sqrt(n_chunks);
    int chunk_size = GRID_WIDTH * GRID_HEIGHT;
    int chunk_side_length = (int) sqrt((double) chunk_size / n_chunks);
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

    int* displacements = malloc(sizeof(int) * n_chunks);
    for (int j=0; j < n_chunks_per_side; j++) {
        for (int i=0; i < n_chunks_per_side; i++) {
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

    int* send_count = malloc(sizeof(int) * n_chunks);
    for (int i = 0; i < n_chunks; i++)
        send_count[i] = 1;

    MPI_Datatype node_type = create_node_datatype();
    MPI_Datatype node_vector_type, node_vector_type_resized;
    MPI_Type_vector(chunk_side_length, chunk_side_length, GRID_WIDTH, node_type, &node_vector_type);
    MPI_Type_commit(&node_vector_type);
    MPI_Type_create_resized(node_vector_type, 0, (MPI_Aint) (chunk_side_length * sizeof(Node)), &node_vector_type_resized);
    MPI_Type_commit(&node_vector_type_resized);

    // Scatter the data
    Node* l_nodes = malloc(sizeof(Node) * chunk_side_length * chunk_side_length);
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

    MPI_Datatype msgStartDatatype = create_MsgStart_datatype();
    if (world_rank == 0) {
        printf("Starting point: %d:%d\n", starting_node->coordinates.x, starting_node->coordinates.y);
        for (int i = 0; i < n_chunks; i++) {
            MsgChunkStart msg = {
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
            find_chunk_corners_exit_points(nodes, chunk_side_length, initial, msg.exit_points);
            find_chunk_sides_exit_points(nodes, chunk_side_length, initial, msg.exit_points);
            if (DEBUG) {
                printf("[DEBUG] R0 - exit_points: ");
                for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                    printf("%d:%d ", msg.exit_points[j].x, msg.exit_points[j].y);
                }
                printf("\n");
            }

            if (MPI_Send(&msg, 1, msgStartDatatype, i, 0, MPI_COMM_WORLD)) {
                printf("[Error]: R0 - Send error\n");
                exit(1);
            }
        }
    }
    MsgChunkStart msg;
    if (MPI_Recv(&msg, 1, msgStartDatatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) {
        printf("[Error]: Recv error\n");
        exit(1);
    }
    if (DEBUG) {
        for (int i = 0; i < n_chunks; i++) {
            if (world_rank == i) {
                printf("Starting point: %d:%d\n", msg.starting_point.x, msg.starting_point.y);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    if (DEBUG) {
        for (int i = 0; i < n_chunks; i++) {
            if (world_rank == i) {
                printf("Rank %d\n", world_rank);
                for (int j = 0; j < chunk_side_length; j++) {
                    for (int k = 0; k < chunk_side_length; k++) {
                        printf("%d:%d", l_nodes[j * chunk_side_length + k].coordinates.x,
                               l_nodes[j * chunk_side_length + k].coordinates.y);
                    }
                    printf("\n");
                }
                printf("\n");
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    free(l_nodes);
    free(displacements);
    free(send_count);
    MPI_Type_free(&node_type);
    MPI_Type_free(&node_vector_type);
    MPI_Type_free(&node_vector_type_resized);
}

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


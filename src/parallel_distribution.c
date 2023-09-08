#include "../include/parallel_distribution.h"
#include "../include/exit_points.h"
#include "../include/cJSON.h"
#include "../include/utility.h"
#include "../include/json_output.h"
#include "../include/parallel_paths.h"

extern int DEBUG;

MPI_Datatype create_coordinates_datatype() {
    MPI_Datatype CoordType;
    MPI_Type_contiguous(2, MPI_INT, &CoordType);
    MPI_Type_commit(&CoordType);

    return CoordType;
}

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

bool is_point_contained(Node* nodes, Coordinates* point, int size){
    int chunk_size = size*size;
    if (DEBUG)
        printf("[DEBUG] Computing id!\n");
    int point_id = point->x*size+point->y;

    int left = 0;
    int right = chunk_size - 1;

    while (left <= right || (point_id <= nodes[right].id && point_id >= nodes[left].id)) {
        int mid = left + (right - left) / 2;

        if (DEBUG){
            printf("[DEBUG] Compare id of point  with node[%d] id!", mid);
            printf("(%d->%d)\n", point_id, nodes[mid].id);
        }
        // Check if the target is at the middle position
        if (nodes[mid].id == point_id) {
            if(DEBUG)
                printf("[DEBUG] FOUND IT!");
            return true;
        }
        // If the target is greater, ignore the left half
        if (nodes[mid].id < point_id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    if(DEBUG)
        printf("[DEBUG] NOT FOUND IT, F**K!");
    return false;
}

void distribute_work(Node *nodes, int size, Node *starting_node, Node *destination_node, int n_chunks, int world_rank) {
    int chunk_side_length, chunk_size, n_chunks_per_side;
    int *send_count;
    int *displacements;
    MPI_Datatype node_type = create_node_datatype();
    MPI_Datatype node_vector_type, node_vector_type_resized;
    if (world_rank == 0) {
        if(DEBUG)
            printf("[DEBUG][PROCESS %d] Matrix dimension: %d\n", world_rank, size);

        if (sqrt(n_chunks) != (int) sqrt(n_chunks)) {
            printf("[ERROR][PROCESS %d] The number of processes must be a perfect square.\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (size * size % n_chunks != 0) {
            printf("[ERROR][PROCESS %d] The total size of the array must be a multiple of the number of processes.\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        n_chunks_per_side = (int) sqrt(n_chunks);
        chunk_size = size * size;
        chunk_side_length = (int) sqrt((double) chunk_size / n_chunks);
        if (chunk_side_length * chunk_side_length * n_chunks != size * size) {
            printf("[ERROR][PROCESS %d] The total size of the array must be a perfect square multiple of the number of processes.\n", world_rank);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        if (DEBUG) {
            printf("[DEBUG][PROCESS 0] Chunk_side_length: %d\n", chunk_side_length);
            printf("[DEBUG][PROCESS 0] N_chunks: %d\n", n_chunks);
        }

        displacements = malloc(sizeof(int) * n_chunks);
        for (int j = 0; j < n_chunks_per_side; j++) {
            for (int i = 0; i < n_chunks_per_side; i++) {
                displacements[j * n_chunks_per_side + i] = (j * n_chunks_per_side * chunk_side_length) + i;
            }
        }

        if (DEBUG) {
            printf("[DEBUG][PROCESS 0] Displacements: ");
            for (int i = 0; i < n_chunks; i++) {
                printf("%d ", displacements[i]);
            }
            printf("\n");
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
        printf("[ERROR][PROCESS %d]: Scatter error\n", world_rank);
        exit(1);
    }

    if (DEBUG)
        printf("[DEBUG][PROCESS %d] Creating MsgStart Datatype!\n", world_rank);
    MPI_Datatype msgStartDatatype = create_MsgStart_datatype();

    if (DEBUG)
        printf("[DEBUG][PROCESS %d] Allocating MsgStart!\n", world_rank);
    MsgChunkStart *messages = malloc(sizeof(MsgChunkStart) * n_chunks);
    MPI_Barrier(MPI_COMM_WORLD);

    if (world_rank == 0) {
#pragma omp parallel for shared(chunk_side_length, nodes)
        for (int i = 0; i < n_chunks; i++) {

            if (DEBUG)
                printf("[DEBUG][PROCESS %d] Creating msg %d", world_rank, i);
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

                if (DEBUG)
                    printf("[DEBUG][PROCESS %d][THREAD %d] Number of exit points: %d\n", world_rank,
                           omp_get_thread_num(), local_msg.num_exit_points);

                if (DEBUG) {
                    printf("[DEBUG][PROCESS %d][THREAD %d] Exit_points: ", world_rank, omp_get_thread_num());
                    for (int j = 0; j < N_EXIT_POINTS_PER_CHUNK; j++) {
                        printf("%d:%d ", local_msg.exit_points[j].x, local_msg.exit_points[j].y);
                    }
                    printf("\n");
                }
            }

            messages[i] = local_msg;
        }
        MPI_Type_free(&node_type);
        MPI_Type_free(&node_vector_type);
        MPI_Type_free(&node_vector_type_resized);
        free(displacements);
        free(send_count);

    }

    MsgChunkStart* msg = malloc(sizeof(MsgChunkStart));

    MPI_Barrier(MPI_COMM_WORLD);
    if(world_rank == 0){

        for (int i = 1; i < n_chunks; ++i) {
            if (DEBUG)
                printf("[DEBUG][PROCESS %d] Sending Message %d\n", world_rank, i);
            if (MPI_Send(&messages[i], 1, msgStartDatatype, i, 0, MPI_COMM_WORLD)) {
                printf("[ERROR][PROCESS 0] Send error\n");
                exit(1);
            }
        }
        if (DEBUG)
            printf("[DEBUG][PROCESS %d] Assigning Message 0\n", world_rank);
        msg = &messages[0];
        if (!is_point_contained(l_nodes, &msg->starting_point, chunk_side_length)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        }if (!is_point_contained(l_nodes, &msg->ending_point, chunk_side_length)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
    } else {
        if (DEBUG)
            printf("[DEBUG][PROCESS %d] Receiving Messages\n", world_rank);

        if (MPI_Recv(msg, 1, msgStartDatatype, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE)) {
            printf("[ERROR][PROCESS %d] Recv error\n", world_rank);
            exit(1);
        }
        if (!is_point_contained(l_nodes, &msg->starting_point, chunk_side_length)){
            msg->starting_point = (Coordinates){.x = -1, .y = -1};
        }if (!is_point_contained(l_nodes, &msg->ending_point, chunk_side_length)){
            msg->ending_point = (Coordinates){.x = -1, .y = -1};
        }
        if (DEBUG) {
            printf("[DEBUG][PROCESS %d] Received Messages\n", world_rank);
            printf("[DEBUG][PROCESS %d]Start: (%d:%d)\n", world_rank, msg->starting_point.x, msg->starting_point.y);
            printf("[DEBUG][PROCESS %d]End: (%d:%d)\n", world_rank, msg->ending_point.x, msg->ending_point.y);
        }
    }

    MsgChunkEnd* computed_paths = parallel_compute_paths(msg, l_nodes, size, world_rank);
    if (DEBUG)
        printf("[DEBUG][PROCESS %d] Found %d paths/%d valid\n", world_rank, computed_paths->num_of_valid_paths, computed_paths->num_of_paths);

    // output_json(nodes, size, starting_node, destination_node, messages, world_rank, n_chunks);
    free(l_nodes);
    MPI_Type_free(&msgStartDatatype);
}
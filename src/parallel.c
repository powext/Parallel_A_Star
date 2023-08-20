#include <printf.h>
#include </usr/local/Cellar/mpich/4.1.2/include/mpi.h>
#include "../include/generic_list.h"
#include "../include/priority_queue.h"
#include "../include/compute_distance.h"
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


void updateNodeValues(Node* node, Node* parent, Node* endNode) {
    // Update the actual distance from the start node
    // node->normal_distance = parent->normal_distance + compute_total_distance(parent, node);

    // Calculate the heuristic distance using a heuristic function (e.g., Euclidean distance)
    node->heuristic_distance = compute_heuristic(*node, *endNode);
}

ChunkPath* compute_path(Node** matrix, int width, int height, Coordinates start, Coordinates end) {
    // Initialize necessary data structures
    PriorityQueue* openSet = createPriorityQueue(width * height);
    int** closedSet = (int**)malloc(height * sizeof(int*));
    Node*** parentMatrix = (Node***)malloc(height * sizeof(Node**));

    for (int i = 0; i < height; i++) {
        closedSet[i] = (int*)calloc(width, sizeof(int));
        parentMatrix[i] = (Node**)malloc(width * sizeof(Node*));
    }

    // Enqueue the starting node
    matrix[start.y][start.x].distance = 0.0;
    matrix[start.y][start.x].heuristic_distance = compute_heuristic(matrix[start.y][start.x], matrix[end.y][end.x]);
    enqueue(openSet, &matrix[start.y][start.x], matrix[start.y][start.x].heuristic_distance);
    for (int i = 0; i < width; i++){
        for (int j= 0; j < height; j++){
            printf("%d %u %f\t", matrix[i][j].id, matrix[i][j].type, matrix[i][j].distance);
        }
        printf("\n");
    }
    while (!isPriorityQueueEmpty(openSet)) {
        for (int i = 0; i < openSet->size; i++){
            printf("%d ", openSet->nodes[i].node->id);
        }
        printf("\n");
        Node* current = dequeue(openSet);
        closedSet[current->coordinates.y][current->coordinates.x] = 1;

        // If the end node is reached, construct the path and return
        if (is_same_node(current->coordinates, end)) {
            // Construct the path using parentMatrix
            ChunkPath* path = (ChunkPath*)malloc(sizeof(ChunkPath));
            path->n_nodes = 0;
            Node* pathNode = current;
            while (pathNode != NULL) {
                path->n_nodes++;
                pathNode = parentMatrix[pathNode->coordinates.y][pathNode->coordinates.x];
            }
            path->nodes = (Coordinates**)malloc(sizeof(Coordinates*) * path->n_nodes);
            path->exit_points = (Coordinates*)malloc(sizeof(Coordinates));
            pathNode = current;
            int index = path->n_nodes - 1;
            while (pathNode != NULL) {
                path->nodes[index] = &(pathNode->coordinates);
                index--;
                Node* temp = pathNode;
                pathNode = parentMatrix[pathNode->coordinates.y][pathNode->coordinates.x];
                if (pathNode != NULL && is_same_node(pathNode->coordinates, end)) {
                    path->exit_points[0] = pathNode->coordinates;
                }
                free(temp);
            }
            destroyPriorityQueue(openSet);
            for (int i = 0; i < height; i++) {
                free(closedSet[i]);
                free(parentMatrix[i]);
            }
            free(closedSet);
            free(parentMatrix);
            for(int i = 0; i < path->n_nodes; i++)
                printf("%d %d", path->nodes[i]->x, path->nodes[i]->y);
            return path;
        }

        // Process neighbors and update openSet
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if ((dx == dy) || (dx == -1 && dy == +1) || (dx == 1 && dy == -1)){
                    continue;
                }
                int neighborX = current->coordinates.x + dx;
                int neighborY = current->coordinates.y + dy;
                if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                    Node* neighbor = &matrix[neighborY][neighborX];
                    printf("%d %u\n", neighbor->id, neighbor->type);
                    if (neighbor->type != obstacle && !closedSet[neighborY][neighborX]) {
                        printf("Checking distance!\n");
                        double tentativeDistance = current->distance + compute_heuristic(*current, *neighbor);
                        printf("%f %f %f %f\n", tentativeDistance, current->distance, compute_heuristic(*current, *neighbor), neighbor->distance);
                        if (tentativeDistance < neighbor->distance) {
                            printf("Updating current node!\n");
                            updateNodeValues(neighbor, current, &matrix[end.y][end.x]);
                            parentMatrix[neighborY][neighborX] = current;
                            enqueue(openSet, neighbor, neighbor->distance + neighbor->heuristic_distance);
                        }
                    }
                }
            }
        }
    }
    printf("priority queue empty!\n");
    // Cleanup and return NULL if path not found
    destroyPriorityQueue(openSet);
    for (int i = 0; i < height; i++) {
        free(closedSet[i]);
        free(parentMatrix[i]);
    }
    free(closedSet);
    free(parentMatrix);
    printf("Returning NULL!\n");
    return NULL;
}

MsgChunkEnd* parallel_compute_paths(MsgChunkStart msg){
    MsgChunkEnd* paths_found = malloc(sizeof(MsgChunkEnd));
    paths_found->paths = malloc(MAX_PATH_NUM*sizeof(ChunkPath));
    for (int i = 0; i < MAX_PATH_NUM; i++){
        paths_found->paths[i] = *(ChunkPath*) NULL;
    }

    if (msg.starting_point != NULL && msg.ending_point != NULL){
        paths_found->paths[0] = *compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, *msg.starting_point, *msg.ending_point);
    }
    else if (msg.starting_point != NULL && msg.ending_point == NULL){
#pragma omp parallel for
        for (int i = 0; i < MAX_EXIT_POINTS; i++){
            if (&msg.exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, *msg.starting_point, msg.exit_points[i]);
                paths_found->paths[i] = *new_path;
            }
        }
    }
    else if (msg.starting_point == NULL && msg.ending_point != NULL){
# pragma omp parallel for
        for (int i = 0; i < MAX_EXIT_POINTS; i++){
            if (&msg.exit_points[i] != NULL){
                ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.exit_points[i], *msg.ending_point);
                paths_found->paths[i] = *new_path;
            }
        }
    }
    else if (msg.starting_point == NULL && msg.ending_point == NULL){
        int p = 0;
        // omp_lock_t lock;
        // omp_init_lock(&lock);
# pragma omp parallel for shared(p, msg, paths_found)
        for (int i = 0; i < MAX_EXIT_POINTS - 1; i++){
            for (int j = i + 1; j < MAX_EXIT_POINTS; j++){
                if (&msg.exit_points[i] != NULL && &msg.exit_points[j] != NULL && i != j){
                    ChunkPath* new_path = compute_path(msg.matrix, msg.chunk_w, msg.chunk_h, msg.exit_points[i], msg.exit_points[j]);
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

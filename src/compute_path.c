//
// Created by Jacopo Clocchiatti on 23/08/23.
//
#include <stdlib.h>

#include "../include/compute_path.h"
#include "../include/compute_distance.h"
#include "../include/priority_queue.h"
#include "../include/utility.h"

#define EDGE_WEIGHT 1
extern int DEBUG;

void updateNodeValues(Node* node, Node* parent, Node* endNode) {
    node->distance = parent->distance + EDGE_WEIGHT;
    node->heuristic = compute_heuristic(*node, *endNode);
    node->score = node->distance + node->heuristic;
}

int compute_new_coords(int old_coord, int reference){
    return old_coord-reference;
}

/* Return ChunkPath in any case, if there is no path the struct
 * will have n_nodes = 0 and nodes (that is the path) = NULL
 */
ChunkPath* compute_path(Node* msg_matrix, int width, int height, Coordinates start, Coordinates end) {
    Node** matrix = (Node**)malloc(sizeof(Node*) * height);
    for (int i = 0; i < height; i++) {
        matrix[i] = (Node*)malloc(width*sizeof(Node));
        for (int j = 0; j < width; ++j) {
            matrix[i][j] = msg_matrix[i*width + j];
        }
    }

    Node *reference = &matrix[0][0];
    Node* starting_node = &matrix[start.y - reference->coordinates.y][start.x - reference->coordinates.x];
    Node* ending_node = &matrix[end.y - reference->coordinates.y][end.x - reference->coordinates.x];

    bool** closedSet = (bool**)malloc(height * sizeof(bool*));
    Node*** parentMatrix = (Node***)malloc(height * sizeof(Node**));

    PriorityQueue* openSet = createPriorityQueue(width);
    for (int i = 0; i < height; i++) {
        closedSet[i] = (bool*)calloc(width, sizeof(bool));
        parentMatrix[i] = (Node**)calloc( width, sizeof(Node*));
    }

    // Enqueue the starting node
    starting_node->distance = 0.0;
    starting_node->heuristic = compute_heuristic(*starting_node, *ending_node);
    starting_node->score = starting_node->distance + starting_node->heuristic;
    enqueue(openSet, starting_node, starting_node->score);

    // Loop until there is not more nodes to check
    while (!isPriorityQueueEmpty(openSet)) {
        Node* current = dequeue(openSet);
        closedSet[current->coordinates.y - reference->coordinates.y][current->coordinates.x - reference->coordinates.x] = true;

        // If the end node is reached, construct the path and return
        if (is_same_node(current->coordinates, end)) {
            // Construct the path using parentMatrix
            ChunkPath* path = (ChunkPath*)malloc(sizeof(ChunkPath));
            path->n_nodes = 0;
            Node* pathNode = current;

            int capacity = 10; // initial capacity, can be any reasonable starting value
            path->nodes = (Coordinates*)malloc(capacity * sizeof(Coordinates));

            while (pathNode != NULL && closedSet[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x]) {
                // Resize if necessary
                if (path->n_nodes == capacity) {
                    capacity *= 2;
                    path->nodes = (Coordinates*)realloc(path->nodes, capacity * sizeof(Coordinates));
                    if(path->nodes == NULL)
                        exit(13);
                }
                path->nodes[path->n_nodes++] = pathNode->coordinates;
                pathNode = parentMatrix[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x];

            }

            // Resize the path->nodes array to the actual path length
            path->nodes = (Coordinates*)realloc(path->nodes, path->n_nodes * sizeof(Coordinates));
            if(path->nodes == NULL)
                exit(13);

            path->exit_points = (Coordinates*)malloc(sizeof(Coordinates)*2);
            path->exit_points[0] = start;
            path->exit_points[1] = end;

            destroyPriorityQueue(openSet);
            for (int i = 0; i < height; i++) {
                free(closedSet[i]);
                free(parentMatrix[i]);
            }
            free(closedSet);
            free(parentMatrix);
            return path;
        }

        // Process neighbors and update openSet
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (abs(dx) == abs(dy)) {
                    continue; // Skip the current node itself or diagonal cell
                }
                int neighbourX = current->coordinates.x + dx - reference->coordinates.x;
                int neighbourY = current->coordinates.y + dy - reference->coordinates.y;
                if (neighbourX >= 0 && neighbourX < width && neighbourY >= 0 && neighbourY < height) {
                    Node* neighbour = &matrix[neighbourY][neighbourX];
                    // printf_m("Processing neighbor: (%d, %d), obstacle: %d, closed: %d\n", neighborX, neighborY, neighbor->type == obstacle, closedSet[neighborY][neighborX]);

                    if (neighbour->type != obstacle && !closedSet[neighbourY][neighbourX]) {
                        double tentativeDistance = current->distance + EDGE_WEIGHT;
                        // printf_debug("tentativeDistance: %f, neighbor->distance: %f\n", tentativeDistance, neighbor->distance);
                        if (tentativeDistance < neighbour->distance) {
                            // printf_debug("Updating node values for: (%d, %d)\n", neighborX, neighborY);
                            updateNodeValues(neighbour, current, ending_node);
                            parentMatrix[neighbourY][neighbourX] = current;
                            enqueue(openSet, neighbour, neighbour->score);
                        }
                    }
                }
            }
        }
    }

    destroyPriorityQueue(openSet);
    for (int i = 0; i < height; i++) {
        free(closedSet[i]);
        free(parentMatrix[i]);
    }
    free(closedSet);
    free(parentMatrix);

    // printf_debug("Path NULL between (%d %d) -> (%d %d)\n", start.x, start.y, end.x, end.y);
    ChunkPath* returned_path = (ChunkPath*)malloc(sizeof(ChunkPath));
    returned_path->n_nodes = 0;
    returned_path->nodes = NULL;
    returned_path->exit_points = (Coordinates*) malloc(sizeof(Coordinates) * 2);
    returned_path->exit_points[0] = start;
    returned_path->exit_points[1] = end;
    return returned_path;
}

//
// Created by Jacopo Clocchiatti on 23/08/23.
//
#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "../include/compute_path.h"
#include "../include/compute_distance.h"
#include "../include/priority_queue.h"

void updateNodeValues(Node* node, Node* parent, Node* endNode) {
    // Update the actual distance from the start node
    node->normal_distance = parent->normal_distance + 1;

    // Calculate the heuristic distance using a heuristic function (e.g., Euclidean distance)
    node->heuristic_distance = compute_heuristic(*node, *endNode);

    node->distance = node->normal_distance + node->heuristic_distance;
}

/* Return ChunkPath in any case, if there is no path the struct
 * will have n_nodes = 0 and nodes (that is the path) = NULL
 */
ChunkPath* compute_path(Node** matrix, int width, int height, Coordinates start, Coordinates end) {
    // Initialize necessary data structures
    PriorityQueue* openSet = createPriorityQueue(width * height);
    int** closedSet = (int**)malloc(height * sizeof(int*));
    NullableNode** parentMatrix = (NullableNode**)malloc(height * sizeof(NullableNode**));

    for (int i = 0; i < height; i++) {
        closedSet[i] = (int*)calloc(width, sizeof(int));
        parentMatrix[i] = (NullableNode*)malloc(width * sizeof(NullableNode*));
    }
    for (int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            parentMatrix[j][i].isNull = true;
        }
    }

    // Enqueue the starting node
    matrix[start.y][start.x].distance = 0.0;
    matrix[start.y][start.x].normal_distance = 0.0;
    matrix[start.y][start.x].heuristic_distance = compute_heuristic(matrix[start.y][start.x], matrix[end.y][end.x]);
    enqueue(openSet, &matrix[start.y][start.x], matrix[start.y][start.x].heuristic_distance);

    while (!isPriorityQueueEmpty(openSet)) {

        Node* current = dequeue(openSet);
        closedSet[current->coordinates.y][current->coordinates.x] = 1;

        // If the end node is reached, construct the path and return
        if (is_same_node(current->coordinates, end)) {
            // Construct the path using parentMatrix
            ChunkPath* path = (ChunkPath*)malloc(sizeof(ChunkPath));
            path->n_nodes = 0;
            NullableNode* pathNode = malloc(sizeof (NullableNode));
            pathNode->node = current;
            pathNode->isNull = false;

            while (!pathNode->isNull) {
                path->n_nodes++;
                pathNode = &parentMatrix[pathNode->node->coordinates.y][pathNode->node->coordinates.x];
            }

            path->nodes = (Coordinates**)malloc(sizeof(Coordinates*) * path->n_nodes);
            path->exit_points = (Coordinates*)malloc(sizeof(Coordinates));
            path->exit_points[0] = start;
            path->exit_points[1] = end;

            pathNode->node = current;
            pathNode->isNull = false;
            int index = path->n_nodes - 1;
            while (!pathNode->isNull && index >= 0) {
                path->nodes[index] = &(pathNode->node->coordinates);
                index--;
                pathNode = &parentMatrix[pathNode->node->coordinates.y][pathNode->node->coordinates.x];
            }
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
                if ((dx == dy) || (dx == -1 && dy == +1) || (dx == 1 && dy == -1)){
                    continue;
                }
                int neighborX = current->coordinates.x + dx;
                int neighborY = current->coordinates.y + dy;
                if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                    Node* neighbor = &matrix[neighborY][neighborX];
                    if (neighbor->type != obstacle && !closedSet[neighborY][neighborX]) {

                        double tentativeDistance = current->normal_distance + compute_heuristic(*current, *neighbor);
                        if (tentativeDistance < neighbor->distance) {
                            updateNodeValues(neighbor, current, &matrix[end.y][end.x]);
                            parentMatrix[neighborY][neighborX].node = current;
                            parentMatrix[neighborY][neighborX].isNull = false;
                            enqueue(openSet, neighbor, neighbor->distance);
                        }
                    }
                }
            }
        }
        // print_matrix(parentMatrix, width, height);
    }

    // Cleanup and return NULL if path not found
    destroyPriorityQueue(openSet);
    for (int i = 0; i < height; i++) {
        free(closedSet[i]);
        free(parentMatrix[i]);
    }
    free(closedSet);
    free(parentMatrix);

    ChunkPath* returned_path = (ChunkPath*)malloc(sizeof(ChunkPath));
    returned_path->n_nodes = 0;
    returned_path->nodes = NULL;
    returned_path->exit_points = (Coordinates*)malloc(sizeof(Coordinates));
    returned_path->exit_points[0] = start;
    returned_path->exit_points[1] = end;
    return returned_path;
}
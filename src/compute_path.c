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
#include "../include/util.h"

#define EDGE_WEIGHT 1

void updateNodeValues(Node* node, Node* parent, Node* endNode) {
    node->distance = parent->distance + EDGE_WEIGHT;

    node->heuristic = compute_heuristic(*node, *endNode);

    node->score = node->distance + node->heuristic;
}

/* Return ChunkPath in any case, if there is no path the struct
 * will have n_nodes = 0 and nodes (that is the path) = NULL
 */
ChunkPath* compute_path(Node** msg_matrix, int width, int height, Coordinates start, Coordinates end) {
    Node** matrix = (Node**)malloc(sizeof(Node*) * height);
    for (int i = 0; i < height; i++) {
        matrix[i] = (Node*)malloc(width*sizeof(Node));
        for (int j = 0; j < width; ++j) {
            matrix[i][j] = msg_matrix[i][j];
        }
    }
    Node* starting_node = &matrix[start.y][start.x];
    Node* ending_node = &matrix[end.y][end.x];

    printf_debug("Finding path"
                 " between (%d %d) -> (%d %d)\n", start.x, start.y, end.x, end.y);
    bool** closedSet = (bool**)malloc(height * sizeof(bool*));
    Node** parentMatrix = (Node**)malloc(height * sizeof(Node**));

    PriorityQueue* openSet = createPriorityQueue(width);
    for (int i = 0; i < height; i++) {
        closedSet[i] = (bool*)calloc(width, sizeof(bool));
        parentMatrix[i] = (Node*)calloc(sizeof(Node*), width);
    }

    // Enqueue the starting node
    starting_node->distance = 0.0;
    starting_node->heuristic = compute_heuristic(*starting_node, *ending_node);
    starting_node->score = starting_node->distance + starting_node->heuristic;
    enqueue(openSet, starting_node, starting_node->score);

    while (!isPriorityQueueEmpty(openSet)) {
        Node* current = dequeue(openSet);
        printf_debug("Dequeued node: (%d, %d)\n", current->coordinates.x, current->coordinates.y);
        closedSet[current->coordinates.y][current->coordinates.x] = true;

        // If the end node is reached, construct the path and return
        if (is_same_node(current->coordinates, end)) {
            printf_debug("End node reached: (%d, %d)\n", end.x, end.y);
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
            path->exit_points = (Coordinates*)malloc(sizeof(Coordinates)*2);
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
                if ((dx == 0 && dy == 0) || (abs(dx) == abs(dy))) {
                    continue; // Skip the current node itself or diagonal cell
                }
                int neighborX = current->coordinates.x + dx;
                int neighborY = current->coordinates.y + dy;
                if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                    Node* neighbor = &matrix[neighborY][neighborX];
                    printf_debug("Processing neighbor: (%d, %d), obstacle: %d, closed: %d\n", neighborX, neighborY, neighbor->type == obstacle, closedSet[neighborY][neighborX]);

                    if (neighbor->type != obstacle && !closedSet[neighborY][neighborX]) {
                        double tentativeDistance = current->distance + EDGE_WEIGHT;
                        printf_debug("tentativeDistance: %f, neighbor->distance: %f\n", tentativeDistance, neighbor->distance);
                        if (tentativeDistance < neighbor->distance) {
                            printf_debug("Updating node values for: (%d, %d)\n", neighborX, neighborY);
                            updateNodeValues(neighbor, current, ending_node);
                            parentMatrix[neighborY][neighborX] = *current;
                            enqueue(openSet, neighbor, neighbor->score);
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

    printf_debug("Path NULL between (%d %d) -> (%d %d)\n", start.x, start.y, end.x, end.y);
    ChunkPath* returned_path = (ChunkPath*)malloc(sizeof(ChunkPath));
    returned_path->n_nodes = 0;
    returned_path->nodes = NULL;
    returned_path->exit_points = (Coordinates*) malloc(sizeof(Coordinates) * 2);
    returned_path->exit_points[0] = start;
    returned_path->exit_points[1] = end;
    return returned_path;
}

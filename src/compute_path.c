//
// Created by Jacopo Clocchiatti on 23/08/23.
//
#include <stdlib.h>

#include "../include/compute_path.h"
#include "../include/priority_queue.h"
#include "../include/utility.h"

#define matrix(i, j) &nodes[(i) * (height) + (j)]

#define NODES_EDGE_WEIGHT 1

extern bool DEBUG;

void updateNodeValues(Node* node, Node* parent, Node* endNode, double (*compute_heuristic)(Node*, Node*)) {
    node->distance = parent->distance + NODES_EDGE_WEIGHT;
    node->heuristic = compute_heuristic(node, endNode);
    node->score = node->distance + node->heuristic;
}

double compute_weight_nodes(Node* node1, Node* node2, AdjList* _) {
    return NODES_EDGE_WEIGHT;
}

double compute_weight_edges(Node* node1, Node* node2, AdjList* adjList) {
    return has_edge(adjList, node1->id, node2);
}

// current, &num_neighbors, matrix, width, height
Node** get_neighbours_nodes(Node* curr, Node* reference, int* n_neighbours, Node* nodes, AdjList* _, int width, int height) {
    Node** neighbours = (Node**)malloc(sizeof(Node*) * 4);
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (abs(dx) == abs(dy)) {
                continue; // Skip the current node itself or diagonal cell
            }
            int neighbourX = curr->coordinates.x + dx - reference->coordinates.x;
            int neighbourY = curr->coordinates.y + dy - reference->coordinates.y;
            if (neighbourX >= 0 && neighbourX < width && neighbourY >= 0 && neighbourY < height) {
                neighbours[*n_neighbours] = matrix(neighbourY, neighbourX);
                (*n_neighbours)++;
            }
        }
    }
    return neighbours;
}

Node** get_neighbours_edges(Node* curr, Node* reference, int* n_neighbours, Node* nodes, AdjList* adjList, int width, int height) {
    int allocated_size = 4;
    Node** neighbours = (Node**)malloc(sizeof(Node*) * allocated_size);
    if (!neighbours) {
        // Handle memory allocation failure
        return NULL;
    }

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (abs(dx) == abs(dy)) {
                continue; // Skip the current node itself or diagonal cell
            }
            int neighbourX = curr->coordinates.x + dx - reference->coordinates.x;
            int neighbourY = curr->coordinates.y + dy - reference->coordinates.y;
            if (
                neighbourX >= 0
                && neighbourX < width
                && neighbourY >= 0
                && neighbourY < height
                && has_any_edge(adjList, curr->id) == 1
            ) {
                neighbours[*n_neighbours] = matrix(neighbourY, neighbourX);
                (*n_neighbours)++;
            }
        }
    }

    Edge* edge = get_index_iterator(adjList, curr->id);
    while (edge != NULL) {
        if (*n_neighbours == allocated_size) {
            allocated_size *= 2;
            neighbours = (Node**)realloc(neighbours, sizeof(Node*) * allocated_size);
            if (!neighbours) {
                printf_debug("Error reallocating memory\n");
                return NULL;
            }
        }
        neighbours[*n_neighbours] = edge->node;
        (*n_neighbours)++;
        edge = edge->next;
    }

    return neighbours;
}

ChunkPath* reassemble_final_path_nodes(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph) {
    ChunkPath* path = (ChunkPath*)malloc(sizeof(ChunkPath));
    path->n_nodes = 0;
    Node* pathNode = current;

    int capacity = 10;
    path->nodes = (Coordinates*)malloc(capacity * sizeof(Coordinates));

    while (pathNode != NULL && closedSet[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x]) {
        if (path->n_nodes == capacity) {
            capacity *= 2;
            path->nodes = (Coordinates*)realloc(path->nodes, capacity * sizeof(Coordinates));
            if(path->nodes == NULL)
                exit(13);
        }
        path->nodes[path->n_nodes++] = pathNode->coordinates;
        pathNode = parentMatrix[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x];
    }

    path->exit_points = (Coordinates*)malloc(sizeof(Coordinates)*2);
    path->exit_points[0] = start;
    path->exit_points[1] = end;
    return path;
}

ChunkPath* reassemble_final_path_edges(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph) {
    ChunkPath* path = (ChunkPath*)malloc(sizeof(ChunkPath));
    path->n_nodes = 0;
    Node* pathNode = current;

    int capacity = 10;
    path->nodes = (Coordinates*)malloc(capacity * sizeof(Coordinates));

    while (pathNode != NULL && closedSet[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x]) {
        if (path->n_nodes == capacity) {
            capacity *= 2;
            path->nodes = (Coordinates*)realloc(path->nodes, capacity * sizeof(Coordinates));
            if(path->nodes == NULL)
                exit(13);
        }
        Node* parent_node = parentMatrix[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x];
        if (parent_node != NULL && has_edge(graph, pathNode->id, parent_node) > -1) {
            ChunkPath* sub_path = get_edge(graph, pathNode->id, parent_node);
            for (int i = 0; i < sub_path->n_nodes; i++) {
                if (path->n_nodes == capacity) {
                    capacity *= 2;
                    path->nodes = (Coordinates*)realloc(path->nodes, capacity * sizeof(Coordinates));
                    if(path->nodes == NULL)
                        exit(13);
                }
                path->nodes[path->n_nodes++] = sub_path->nodes[i];
            }
            pathNode = parent_node;
        } else {
            path->nodes[path->n_nodes++] = pathNode->coordinates;
            pathNode = parentMatrix[pathNode->coordinates.y - reference->coordinates.y][pathNode->coordinates.x - reference->coordinates.x];
        }
    }

    path->exit_points = (Coordinates*)malloc(sizeof(Coordinates)*2);
    path->exit_points[0] = start;
    path->exit_points[1] = end;
    return path;
}


/* Return ChunkPath in any case, if there is no path the struct
 * will have n_nodes = 0 and nodes (that is the path) = NULL
 */
ChunkPath* compute_path(
        Node* nodes,
        AdjList *graph,
        int width,
        int height,
        Coordinates start,
        Coordinates end,
        double (*compute_weight)(Node* a, Node* b, AdjList* graph),
        double (*compute_heuristic)(Node* a, Node* b),
        Node** (*get_neighbors)(Node* curr, Node* reference, int* n_neighbours, Node* nodes, AdjList* graph, int width, int height),
        ChunkPath* (*reassemble_final_path)(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph)
) {
    Node* reference = matrix(0, 0);
    Node* starting_node = matrix(start.y - reference->coordinates.y, start.x - reference->coordinates.x);
    Node* ending_node = matrix(end.y - reference->coordinates.y, end.x - reference->coordinates.x);

    bool** closedSet = (bool**)malloc(height * sizeof(bool*));
    Node*** parentMatrix = (Node***)malloc(height * sizeof(Node**));

    PriorityQueue* openSet = createPriorityQueue(width);
    for (int i = 0; i < height; i++) {
        closedSet[i] = (bool*)calloc(width, sizeof(bool));
        parentMatrix[i] = (Node**)calloc( width, sizeof(Node*));
    }

    // debug Coordinates start and end
    printf_debug("Start: (%d, %d)\n", start.x, start.y);

    // Enqueue the starting node
    starting_node->distance = 0.0;
    starting_node->heuristic = compute_heuristic(starting_node, ending_node);
    starting_node->score = starting_node->distance + starting_node->heuristic;
    enqueue(openSet, starting_node, starting_node->score);

    while (!isPriorityQueueEmpty(openSet)) {
        Node* current = dequeue(openSet);
        // debug current
        printf_debug("Current node: (%d, %d)\n", current->coordinates.x, current->coordinates.y);
        closedSet[current->coordinates.y - reference->coordinates.y][current->coordinates.x - reference->coordinates.x] = true;

        if (is_same_node(current->coordinates, end)) {
            ChunkPath* path = reassemble_final_path(current, reference, start, end, closedSet, parentMatrix, nodes, graph);

            destroyPriorityQueue(openSet);
            for (int i = 0; i < height; i++) {
                free(closedSet[i]);
                free(parentMatrix[i]);
            }
            free(closedSet);
            free(parentMatrix);
            return path;
        }

        int num_neighbors = 0;
        Node** neighbors = get_neighbors(current, reference, &num_neighbors, nodes, graph, width, height);
        for (int i = 0; i < num_neighbors; i++) {
            Node* neighbour = neighbors[i];
            int neighbourX = neighbour->coordinates.x - reference->coordinates.x;
            int neighbourY = neighbour->coordinates.y - reference->coordinates.y;

            // debug neighbour
            printf_debug("Neighbour node: (%d, %d)\n", neighbour->coordinates.x, neighbour->coordinates.y);

            if (neighbour->type != obstacle && !closedSet[neighbourY][neighbourX]) {
                //debug
                printf_debug("Neighbour node is not obstacle and not in closed set\n");
                double tentativeDistance = current->distance + compute_weight(current, neighbour, graph);
                // debug all the values
                printf_debug("Tentative distance: %f\n", tentativeDistance);
                printf_debug("Neighbour distance: %f\n", neighbour->distance);
                printf_debug("Neighbour heuristic: %f\n", neighbour->heuristic);
                printf_debug("Neighbour score: %f\n", neighbour->score);
                if (tentativeDistance < neighbour->distance) {
                    updateNodeValues(neighbour, current, ending_node, compute_heuristic);
                    parentMatrix[neighbourY][neighbourX] = current;
                    enqueue(openSet, neighbour, neighbour->score);
                }
            }
        }
        free(neighbors);
    }

    printf_debug("No path found\n");

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
    returned_path->exit_points = (Coordinates*) malloc(sizeof(Coordinates) * 2);
    returned_path->exit_points[0] = start;
    returned_path->exit_points[1] = end;
    return returned_path;
}

//
// Created by Jacopo Clocchiatti on 23/08/23.
//

#ifndef PARALLEL_A_STAR_NEW_COMPUTE_PATH_H
#define PARALLEL_A_STAR_NEW_COMPUTE_PATH_H


#include <stdbool.h>
#include "comm.h"
#include "adjlist.h"

ChunkPath* compute_path(
        Node* nodes,
        AdjList* graph,
        int width,
        int height,
        Coordinates start,
        Coordinates end,
        double (*compute_weight)(Node* a, Node* b, AdjList* graph),
        double (*compute_heuristic)(Node* a, Node* b),
        Node** (*get_neighbors)(Node* curr, Node* reference, int* n_neighbours, Node* nodes, AdjList* graph, int width, int height),
        ChunkPath* (*reassemble_final_path)(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph)
);
double compute_weight_nodes(Node* node1, Node* node2, AdjList* graph);
double compute_weight_edges(Node* node1, Node* node2, AdjList* graph);
Node **get_neighbours_nodes(Node *curr, Node* reference, int *n_neighbours, Node *nodes, AdjList* graph, int width, int height);
Node **get_neighbours_edges(Node *curr, Node* reference, int *n_neighbours, Node *nodes, AdjList* adjList, int width, int height);
ChunkPath* reassemble_final_path_nodes(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph);
ChunkPath* reassemble_final_path_edges(Node* current, Node* reference, Coordinates start, Coordinates  end, bool** closedSet, Node*** parentMatrix, Node* nodes, AdjList* graph);

#endif //PARALLEL_A_STAR_NEW_COMPUTE_PATH_H

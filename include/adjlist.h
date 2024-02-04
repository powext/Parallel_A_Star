//
// Created by Simone Bianchin on 16/09/23.
//

#ifndef PARALLEL_A_STAR_NEW_ADJLIST_H
#define PARALLEL_A_STAR_NEW_ADJLIST_H

#include <stdbool.h>
#include "comm.h"

typedef struct Edge {
    Node* node;
    int weight;
    ChunkPath* path;
    struct Edge* next;
} Edge;

typedef struct {
    Node* node;
    Edge* edges;
} AdjList;

// Function prototypes
AdjList* create_graph(int num_nodes);
void add_edge(AdjList* graph, int start_index, Node* end_node, ChunkPath* path);
Edge* get_index_iterator(AdjList* graph, int start_index);
int has_edge(AdjList* graph, int start_index, Node* end_node);
ChunkPath* get_edge(AdjList* graph, int start_index, Node* end_node);
int has_any_edge(AdjList* graph, int start_index);
void free_graph(AdjList* graph, int num_nodes);

#endif //PARALLEL_A_STAR_NEW_ADJLIST_H

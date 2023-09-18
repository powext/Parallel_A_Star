#include "../include/adjlist.h"
#include <stdlib.h>

AdjList* create_graph(int num_nodes) {
    AdjList* graph = (AdjList*) malloc(num_nodes * sizeof(AdjList));
    for (int i = 0; i < num_nodes; i++) {
        graph[i].node = NULL;
        graph[i].edges = NULL;
    }
    return graph;
}

void add_edge(AdjList* graph, int start_index, Node* end_node, ChunkPath* path) {
    Edge* new_edge = (Edge*) malloc(sizeof(Edge));
    new_edge->node = end_node;
    new_edge->weight = path->n_nodes;
    new_edge->path = path;
    new_edge->next = graph[start_index].edges;
    graph[start_index].edges = new_edge;
}

int has_edge(AdjList* graph, int start_index, Node* end_node) {
    Edge* edge = graph[start_index].edges;
    while (edge) {
        if (edge->node->id == end_node->id) {
            return edge->weight;
        }
        edge = edge->next;
    }
    return -1;
}

ChunkPath* get_edge(AdjList* graph, int start_index, Node* end_node) {
    Edge* edge = graph[start_index].edges;
    while (edge) {
        if (edge->node->id == end_node->id) {
            return edge->path;
        }
        edge = edge->next;
    }
    return NULL;
}

int has_any_edge(AdjList* graph, int start_index) {
    return graph[start_index].edges != NULL;
}

Edge* get_index_iterator(AdjList* graph, int start_index) {
    return graph[start_index].edges;
}

void free_graph(AdjList* graph, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        Edge* edge = graph[i].edges;
        while (edge) {
            Edge* temp = edge;
            edge = edge->next;
            free(temp);
        }
    }
    free(graph);
}

//
// Created by Simone Bianchin on 04/02/23.
//

#ifndef PARALLEL_A_STAR_PRIORITY_QUEUE_H
#define PARALLEL_A_STAR_PRIORITY_QUEUE_H

#include <stdbool.h>
#include "hash_table.h"
#include "node.h"
#include <malloc/_malloc.h>
#include <stddef.h>
#include <stdio.h>

typedef struct MinHeap MinHeap;

struct MinHeap{
    Node** arr;
    int used;
    int capacity;
    HashTable positions;
};

MinHeap* init_minheap();
bool is_queue_empty(MinHeap* heap);
MinHeap* insert_into_heap(MinHeap* heap, Node* node);
Node* pop_min(MinHeap* heap);
Node* get_node(MinHeap* heap, Node* node);
bool find_in_heap(MinHeap* list, Node* node);
void free_minheap(MinHeap* heap);


#endif //PARALLEL_A_STAR_PRIORITY_QUEUE_H

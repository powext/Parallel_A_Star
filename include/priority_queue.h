//
// Created by Simone Bianchin on 04/02/23.
//

#ifndef PARALLEL_A_STAR_PRIORITY_QUEUE_H
#define PARALLEL_A_STAR_PRIORITY_QUEUE_H

#include <stdbool.h>
#include "hash_table.h"
#include "comm.h"
#include <malloc/_malloc.h>
#include <stddef.h>
#include <stdio.h>

PriorityQueue* createPriorityQueue(int initial_capacity);
void enqueue(PriorityQueue* pq, Node* node, double priority);
Node* dequeue(PriorityQueue* pq);
int isPriorityQueueEmpty(PriorityQueue* pq);
void destroyPriorityQueue(PriorityQueue* pq);
bool is_same_node(Coordinates a, Coordinates b);

#endif //PARALLEL_A_STAR_PRIORITY_QUEUE_H

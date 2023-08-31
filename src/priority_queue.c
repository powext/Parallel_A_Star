//
// Created by Jacopo Clocchiatti on 01/12/22.
//

#include <stdbool.h>
#import <stdlib.h>
#include "../include/comm.h"

bool is_same_node(Coordinates a, Coordinates b){
    return (a.x == b.x && a.y == b.y);
}

int isAlreadyPresent(PriorityQueue* pq, Node* node){
    if(pq->size > 0){
        for (int i = 0; i < pq->size; i++){
            if(is_same_node(pq->nodes[i].node->coordinates, node->coordinates)){
                return i;
            }
        }
    }
    return -1;
}

PriorityQueue* createPriorityQueue(int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->size = 0;
    pq->capacity = capacity;
    pq->nodes = (PriorityQueueNode*)malloc(sizeof(PriorityQueueNode) * capacity);
    return pq;
}

void enqueue(PriorityQueue* pq, Node* node, double priority) {
    if (pq->size >= pq->capacity) {
        // Resize the priority queue by increasing its capacity by half
        int newCapacity = pq->capacity + pq->capacity / 2;
        PriorityQueueNode* newNodes = (PriorityQueueNode*)malloc(sizeof(PriorityQueueNode) * newCapacity);

        // Copy existing nodes to the new array
        for (int i = 0; i < pq->size; i++) {
            newNodes[i] = pq->nodes[i];
        }

        // Free the old nodes array and update the queue's properties
        free(pq->nodes);
        pq->nodes = newNodes;
        pq->capacity = newCapacity;
    }

    int currentIndex;
    int pos;
    pos = isAlreadyPresent(pq, node);
    if(pos >= 0){
        if (pq->nodes[pos].priority > priority){
            pq->nodes[pos].priority = priority;
            currentIndex = pos;
        } else {
            return;
        }
    } else {
        // Create a new PriorityQueueNode
        PriorityQueueNode newNode;
        newNode.node = node;
        newNode.priority = priority;

        // Add the new node to the end of the priority queue
        pq->nodes[pq->size] = newNode;
        pq->size++;

        // Bubble up the new node to maintain the heap property
        currentIndex = pq->size - 1;
    }

    while (currentIndex > 0) {
        int parentIndex = (currentIndex - 1) / 2;
        if (pq->nodes[currentIndex].priority < pq->nodes[parentIndex].priority) {
            // Swap nodes if the child's priority is smaller than its parent's
            PriorityQueueNode temp = pq->nodes[currentIndex];
            pq->nodes[currentIndex] = pq->nodes[parentIndex];
            pq->nodes[parentIndex] = temp;
            currentIndex = parentIndex;
        } else {
            break;  // Heap property is maintained
        }
    }
}

Node* dequeue(PriorityQueue* pq) {
    if (pq->size == 0) {
        return NULL;  // Queue is empty
    }

    // Get the node with the highest priority (at the root of the heap)
    Node* highestPriorityNode = pq->nodes[0].node;

    // Replace the root node with the last node in the heap
    pq->nodes[0] = pq->nodes[pq->size - 1];
    pq->size--;

    // Heapify down to maintain the min-heap property
    int currentIndex = 0;
    while (1) {
        int leftChildIndex = 2 * currentIndex + 1;
        int rightChildIndex = 2 * currentIndex + 2;
        int smallestIndex = currentIndex;

        if (leftChildIndex < pq->size && pq->nodes[leftChildIndex].priority < pq->nodes[smallestIndex].priority) {
            smallestIndex = leftChildIndex;
        }
        if (rightChildIndex < pq->size && pq->nodes[rightChildIndex].priority < pq->nodes[smallestIndex].priority) {
            smallestIndex = rightChildIndex;
        }

        if (smallestIndex != currentIndex) {
            // Swap nodes to maintain the min-heap property
            PriorityQueueNode temp = pq->nodes[currentIndex];
            pq->nodes[currentIndex] = pq->nodes[smallestIndex];
            pq->nodes[smallestIndex] = temp;
            currentIndex = smallestIndex;
        } else {
            break;  // Heap property is maintained
        }
    }

    return highestPriorityNode;
}

int isPriorityQueueEmpty(PriorityQueue* pq) {
    return pq->size == 0;
}

void destroyPriorityQueue(PriorityQueue* pq) {
    free(pq->nodes);
    free(pq);
}

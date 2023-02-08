//
// Created by Jacopo Clocchiatti on 01/12/22.
//

// TODO: implement hash into functions + finish update priority

#include "../include/priority_queue.h"


int parent(int i){
    return (i-1)/2;
}

int left_child(int i){
    return (2*i + 1);
}

int right_child(int i){
    return(2*i + 2);
}


MinHeap* init_minheap(int capacity){
    MinHeap* minheap = (MinHeap*) calloc(1, sizeof(MinHeap));
    if (minheap == NULL){
        fprintf(stderr, "Error allocating the minheap");
    }
    minheap->arr = (Node**) calloc(capacity+1, sizeof(Node*));
    if (minheap->arr == NULL){
        fprintf(stderr, "Error allocating the minheap array");
    }
    minheap->capacity = capacity;
    minheap->size = 0;
    return minheap;
}

void free_minheap(MinHeap* heap) {
    if (!heap)
        return;
    free(heap->arr);
    free(heap);
}

MinHeap* insert_into_heap(MinHeap* heap, Node node) {
    // Check if we need to resize the array
    if (heap->size == heap->capacity) {
        heap->capacity *= 2;
        heap->arr = (Node**) realloc(heap->arr, heap->capacity * sizeof(Node*));
    }
    // We can add it. Increase the size and add it to the end
    heap->arr[heap->size] = &node;
    heap->size++;

    // Keep swapping until we reach the root
    int curr = heap->size - 1;
    // As long as you aren't in the root node, and while the
    // parent of the last element is greater than it
    // todo: choose if last inserted should be checked before or after the nodes already in the heap
    while (curr > 0 && heap->arr[parent(curr)]->distance > heap->arr[curr]->distance) {
        // Swap
        Node temp = *heap->arr[parent(curr)];
        heap->arr[parent(curr)] = heap->arr[curr];
        heap->arr[curr] = &temp;
        // Update the current index of element
        curr = parent(curr);
    }
    return heap;
}

bool is_queue_empty(MinHeap* heap){
    if (heap->size <= 0){
        return true;
    }
    return false;
}

Node get_min(MinHeap* heap){
    return *heap->arr[0];
}

void heapify(MinHeap* heap, int i) {
    int smallest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    // If left child is smaller than root
    if (left < heap->size && heap->arr[left]->distance < heap->arr[smallest]->distance)
        smallest = left;

    // If right child is smaller than root
    if (right < heap->size && heap->arr[right]->distance < heap->arr[smallest]->distance)
        smallest = right;

    // If root is not smallest, swap with smallest and call heapify again
    if (smallest != i) {
        Node temp = *heap->arr[i];
        heap->arr[i] = heap->arr[smallest];
        heap->arr[smallest] = &temp;
        heapify(heap, smallest);
    }
}

Node pop_min(MinHeap* heap) {
    Node min = *heap->arr[0];
    heap->arr[0] = heap->arr[heap->size - 1];
    heap->size--;

    // Call heapify on root node
    heapify(heap, 0);

    return min;
}

// todo: better find alg
bool find(Node** list, int list_len,  Node* node) {
    for (int i = 0; i < list_len; i++){
        if (list[i]->id == node->id){
            return true;
        }
    }
    return false;
}

Node* get_node(MinHeap* heap, Node* node) {
    for (int i = 0; i < heap->size; i++){
        if (heap->arr[i]->id == node->id){
            return heap->arr[i];
        }
    }

    return NULL;
}

MinHeap* update_priority(MinHeap* heap, Node* node){
    // todo: update_priority
    return heap;
}
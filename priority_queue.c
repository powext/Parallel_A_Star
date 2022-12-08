//
// Created by Jacopo Clocchiatti on 01/12/22.
//

// TODO: implement hash into functions + finish update priority +
#include "priority_queue.h"

typedef struct MinHeap MinHeap;

struct MinHeap{
    Node* arr;
    int size;
    int capacity;
    HashTable positions;
};


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
    minheap->arr = (Node*) calloc(capacity+1, sizeof(Node));
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

    if (heap->size == heap->capacity) {
        fprintf(stderr, "Cannot insert new node. Heap is already full!\n");
        return heap;
    }
    // We can add it. Increase the size and add it to the end
    heap->size++;
    heap->arr[heap->size - 1] = node;

    // Keep swapping until we reach the root
    int curr = heap->size - 1;
    // As long as you aren't in the root node, and while the
    // parent of the last element is greater than it
    // todo: choose if last inserted should be checked before or after the nodes already in the heap
    while (curr > 0 && heap->arr[parent(curr)].distance > heap->arr[curr].distance) {
        // Swap
        Node temp = heap->arr[parent(curr)];
        heap->arr[parent(curr)] = heap->arr[curr];
        heap->arr[curr] = temp;
        // Update the current index of element
        curr = parent(curr);
    }
    return heap;
}

bool is_empty(MinHeap* heap){
    if (heap->size <= 0){
        return true;
    }
    return false;
}

Node get_min(MinHeap* heap){
    return heap->arr[0];
}

Node delete_min(MinHeap* heap){
    if (heap->size == 0){
        fprintf(stderr, "No element to pop");
    }

    Node min = heap->arr[0];
    heap->arr[0] = heap->arr[heap->size-1];
    heap->size--;
    return min;
}

// todo: better find alg
bool find(Node* list, int list_len,  Node* node){
    for (int i = 0; i < list_len; i++){
        if (list[i].value == node->value){
            return true;
        }
    }
    return false;
}

Node* get_node(MinHeap* heap, Node* node){
    for (int i = 0; i < heap->size; i++){
        if (heap->arr[i].value == node->value){
            return &heap->arr[i];
        }
    }

    return NULL;
}

MinHeap* update_priority(MinHeap* heap, Node* node){
    // todo: update_priority
    return heap;
}
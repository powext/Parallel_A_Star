//
// Created by Simone Bianchin on 04/02/23.
//

#ifndef PARALLEL_A_STAR_LIST_H
#define PARALLEL_A_STAR_LIST_H

#include <malloc/_malloc.h>
#include <printf.h>
#include <stdbool.h>
#include "../include/node.h"
#include "comm.h"

typedef struct List{
    void** arr;
    int used;
    int length;
} List;

typedef struct LinkedNode {
    Node* node;
    struct LinkedNode* next;
} LinkedNode;

List* init_list();
void insert_into_list(List* list, void* node);
void free_list(List* list);
bool find_in_list(List* list, Node* node);

#endif //PARALLEL_A_STAR_LIST_H

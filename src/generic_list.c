//
// Created by Jacopo Clocchiatti on 07/12/22.
//

#include "../include/generic_list.h"
#include "../include/comm.h"

List* init_list() {
    List* list = (List*) malloc(sizeof(List));
    list->arr = (void**) malloc(sizeof(void*));
    list->used = 0;
    list->length = 1;
    return list;
}

void free_list(List* list) {
    if (!list)
        return;
    free(list->arr);
    free(list);
}

void insert_into_list(List* list, void* node) {
    if (list->used == list->length) {
        list->length *= 2;
        list->arr = realloc(list->arr, list->length * sizeof(void*));
    }

    list->arr[list->used] = node;
    list->used++;
}

// todo: better find_in_heap alg
bool find_in_list(List* list,  Node* node) {
    for (int i = 0; i < list->used; i++){
        if (((Node*) list->arr[i])->id == node->id){
            return true;
        }
    }
    return false;
}

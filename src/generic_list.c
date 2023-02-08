//
// Created by Jacopo Clocchiatti on 07/12/22.
//

#include "../include/generic_list.h"

typedef struct List List;

List* init_list() {
    List* list = (List*) calloc(1, sizeof(List));
    list->arr = (void**) calloc(1, sizeof(void*));
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

List* insert_into_list(List* list, void* node) {
    if (list->used == list->length) {
        list->length *= 2;
        list->arr = realloc(list->arr, list->length * sizeof(void*));
    }

    list->arr[list->used] = node;
    list->used++;
    return list;
}

// todo: better find alg
// list is not ordered, linear scan might be best case
bool find_in_list(Node* list, int list_len,  Node* node){
    for (int i = 0; i < list_len; i++){
        if (list[i].id == node->id){
            return true;
        }
    }
    return false;
}
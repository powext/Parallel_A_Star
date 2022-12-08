//
// Created by Jacopo Clocchiatti on 07/12/22.
//

#include "list.h"

typedef struct List List;

struct List{
    Node* arr;
    int len;
};


List* init_list(int max_len){
    List* list = (List*) calloc(1, sizeof(List));
    list->arr = (Node*) calloc(max_len, sizeof (Node));
    list->len = 0;
    return list;
}

void free_list(List* list) {
    if (!list)
        return;
    free(list->arr);
    free(list);
}

List* insert_into_list(List* list, Node* node){
    list->len++;
    list->arr[list->len-1] = *node;
    return list;
}

void print_list(List* list){
    for (int i=0; i<list->len; i++){
        printf("%c", list->arr[i].value);
    }
}

// todo: better find alg
// list is not ordered, linear scan might be best case
bool find(Node* list, int list_len,  Node* node){
    for (int i = 0; i < list_len; i++){
        if (list[i].value == node->value){
            return true;
        }
    }
    return false;
}
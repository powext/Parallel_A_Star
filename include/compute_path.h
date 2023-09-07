//
// Created by Jacopo Clocchiatti on 23/08/23.
//

#ifndef PARALLEL_A_STAR_NEW_COMPUTE_PATH_H
#define PARALLEL_A_STAR_NEW_COMPUTE_PATH_H


#include <stdbool.h>
#include "comm.h"

typedef struct NullableNode{
    Node* node;
    bool isNull;
} NullableNode;

void updateNodeValues(Node* node, Node* parent, Node* endNode);
ChunkPath* compute_path(Node* matrix, int width, int height, Coordinates start, Coordinates end);

#endif //PARALLEL_A_STAR_NEW_COMPUTE_PATH_H

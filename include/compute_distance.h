//
// Created by Simone Bianchin on 04/02/23.
//

#ifndef PARALLEL_A_STAR_COMPUTE_DISTANCE_H
#define PARALLEL_A_STAR_COMPUTE_DISTANCE_H

#include <stdbool.h>
#include "node.h"

double compute_total_distance();
double compute_heuristic(Node a, Node b);
bool find(Node** list, int list_len,  Node* node);

#endif //PARALLEL_A_STAR_COMPUTE_DISTANCE_H

//
// Created by Jacopo Clocchiatti on 01/12/22.
//

#include <stdlib.h>
#include "../include/compute_distance.h"

/*
 * Manhattan distance
 * possible alternative Euclidean distance √(x1-x2)^2+(y1-y2)^2
 */
double compute_heuristic_nodes(Node* a, Node* b) {
    int dx = abs(a->coordinates.x - b->coordinates.x);
    int dy = abs(a->coordinates.y - b->coordinates.y);
    return (double) (dx + dy);
}

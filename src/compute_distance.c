//
// Created by Jacopo Clocchiatti on 01/12/22.
//

#include <stdlib.h>
#include "../include/compute_distance.h"

double compute_heuristic(Node a, Node b){
    int dx = abs(a.coordinates.x - b.coordinates.x);
    int dy = abs(a.coordinates.y - b.coordinates.y);
    return (double) (dx + dy);
}

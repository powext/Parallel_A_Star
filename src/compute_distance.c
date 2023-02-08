//
// Created by Jacopo Clocchiatti on 01/12/22.
//


#include <stdlib.h>
#include "../include/compute_distance.h"

double compute_heuristic(Node a, Node b){
    return abs(a.x - b.x) + abs(a.y - b.y);
}

double compute_classic_distance(){
    // TODO
    return 0.0;
}

double compute_total_distance(){
    // double classic_distance = compute_classic_distance();

    // double heuristic = compute_heuristic();

    // double distance = classic_distance + heuristic;

    return 0;
}
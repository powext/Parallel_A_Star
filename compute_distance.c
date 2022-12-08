//
// Created by Jacopo Clocchiatti on 01/12/22.
//


double compute_heuristic(){
    // TODO
    return 0.0;
}

double compute_classic_distance(){
    // TODO
    return 0.0;
}

double compute_total_distance(){
    double classic_distance = compute_classic_distance();

    double heuristic = compute_heuristic();

    double distance = classic_distance + heuristic;

    return distance;
}
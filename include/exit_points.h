#include <tic.h>
#include "../include/print.h"

#ifndef PARALLEL_A_STAR_NEW_EXIT_POINTS_H
#define PARALLEL_A_STAR_NEW_EXIT_POINTS_H

void find_chunk_corners_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points);
void find_chunk_sides_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points);

#endif //PARALLEL_A_STAR_NEW_EXIT_POINTS_H

#include <tic.h>
#include "../include/print.h"

#ifndef PARALLEL_A_STAR_NEW_EXIT_POINTS_H
#define PARALLEL_A_STAR_NEW_EXIT_POINTS_H

int find_chunk_corners_exit_points(int rank, Node* nodes, int size, int chunk_side_length, Coordinates initial, Coordinates *exit_points);
int find_chunk_sides_exit_points(int rank, Node* nodes, int size, int chunk_side_length, Coordinates initial, Coordinates *exit_points);

#endif //PARALLEL_A_STAR_NEW_EXIT_POINTS_H

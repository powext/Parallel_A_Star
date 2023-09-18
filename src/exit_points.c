#include <tic.h>
#include "../include/print.h"
#include "../include/utility.h"

// extern int GRID_HEIGHT;
// extern int GRID_WIDTH;
extern int DEBUG;
extern int DEBUG_PROCESS;

int find_chunk_corners_exit_points(int rank, Node *nodes, int size, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    enum Direction {
        up, down, left, right
    };
    struct Angle {
        Coordinates coordinates;
        enum Direction directions[2];
    };
    struct Angle corners[4] = {
            {initial,                           {up, left}},
            {initial.x + chunk_side_length - 1, initial.y,                         {up,   right}},
            {initial.x + chunk_side_length - 1, initial.y + chunk_side_length - 1, {down, right}},
            {initial.x,                         initial.y + chunk_side_length - 1, {down, left}}
    };

    int number_of_exit_points = 0;

    for (int i = 0; i < 4; i++) {
        Coordinates corner = corners[i].coordinates;
        Node *corner_node = &nodes[corner.y * size + corner.x];
        enum Direction *directions = corners[i].directions;
        printf_debug("Corner: %d:%d\n", corner.x, corner.y);
        if (corner_node->type == obstacle) {
            printf_debug("Corner is an obstacle\n");
            continue;
        }
        for (int j = 0; j < 2; j++) {
            enum Direction direction = directions[j];
            printf_debug("Direction: %d\n", direction);
            Coordinates outlet_coords = corner;
            switch (direction) {
                case up:
                    outlet_coords.y = corner.y - 1;
                    break;
                case down:
                    outlet_coords.y = corner.y + 1;
                    break;
                case left:
                    outlet_coords.x = corner.x - 1;
                    break;
                case right:
                    outlet_coords.x = corner.x + 1;
                    break;
            }
            printf_debug("Exit point: %d:%d\n", outlet_coords.x, outlet_coords.y);
            if (outlet_coords.x < 0 || outlet_coords.x >= size || outlet_coords.y < 0 ||
                outlet_coords.y >= size) {
                printf_debug("Exit point out of bounds\n");
                continue;
            }
            Node *outlet_node = &nodes[outlet_coords.y * size + outlet_coords.x];
            if (outlet_node->type == obstacle) {
                printf_debug("Exit point is an obstacle\n");
                continue;
            }
            Node *exit_point = &nodes[corner.y * size + corner.x];
            exit_points[i] = exit_point->coordinates;
            number_of_exit_points += 1;
            printf_debug("Exit point is valid\n");
        }
    }

    return 4;
}

int find_exit_points_on_vector(int world_rank, Node *nodes, int size, Coordinates *vector, Coordinates *outer_vector, int vector_length,
                               Coordinates *exit_points, int direction) {
    int tmp_length = 0;
    if (DEBUG) {
        printf_debug("Searching new vector: ");
        for (int i = 0; i < vector_length; i++) {
            if (!DEBUG) continue;
            if (DEBUG_PROCESS > 0 && DEBUG_PROCESS != world_rank) continue;
            printf("%d:%d ", vector[i].x, vector[i].y);
        }
        printf_debug("\n");
        printf_debug("Searching new outer_vector: ");
        for (int i = 0; i < vector_length; i++) {
            if (!DEBUG) continue;
            if (DEBUG_PROCESS > 0 && DEBUG_PROCESS != world_rank) continue;
            printf("%d:%d ", outer_vector[i].x, outer_vector[i].y);
        }
        printf_debug("\n");
    }
    int N = (N_EXIT_POINTS_PER_CHUNK - 4) / 4;
    int step = (vector_length - 2) / N; // Excludes the first and last entry

    for (int i = 1; i <= N; i++) {
        int index = 1 + (i * step) - (step / 2);

        // Check the middle point first
        if (nodes[outer_vector[index].y * size + outer_vector[index].x].type != obstacle
            && nodes[vector[index].y * size + vector[index].x].type != obstacle) {
            exit_points[4 + (direction * N) + i - 1] = vector[index];
            tmp_length += 1;
            printf_debug("Side exit point found: %d:%d\n", vector[index].x, vector[index].y);
            continue;
        }

        // Searching to the left from the middle
        int left_index = index - 1;
        while (left_index > 1) {
            if (nodes[outer_vector[left_index].y * size + outer_vector[left_index].x].type != obstacle
                && nodes[vector[left_index].y * size + vector[left_index].x].type != obstacle) {
                exit_points[4 + (direction * N) + i - 1] = vector[left_index];
                tmp_length += 1;
                printf_debug("Side exit point found: %d:%d\n", vector[left_index].x, vector[left_index].y);
                left_index = 0;  // exit cycle
            } else {
                left_index--;
                continue;
            }
        }
        if (left_index == 0) continue;

        // If not found in the middle or to the left, searching to the right
        int right_index = index + 1;
        while (right_index < vector_length - 1) {
            if (nodes[outer_vector[right_index].y * size + outer_vector[right_index].x].type != obstacle
                && nodes[vector[right_index].y * size + vector[right_index].x].type != obstacle) {
                exit_points[4 + (direction * N) + i - 1] = vector[right_index];
                tmp_length += 1;
                printf_debug("Side exit point found: %d:%d\n", vector[right_index].x, vector[right_index].y);
                right_index = vector_length; // exit cycle
            }
            else {
                right_index++;
            }
        }
    }
    return tmp_length;
}

int find_chunk_sides_exit_points(int rank, Node *nodes, int size, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    Coordinates vector1a[chunk_side_length];
    Coordinates vector1b[chunk_side_length];
    Coordinates vector2a[chunk_side_length];
    Coordinates vector2b[chunk_side_length];

    int number_of_exit_points = 0;
    printf_debug("Initial: %d:%d\n", initial.x, initial.y);

    // Iterating top and bottom
    if ((initial.y - 1) >= 0) {
        printf_debug("Ok top\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[initial.y * size + initial.x + i].coordinates;
            vector1b[i] = nodes[(initial.y - 1) * size + initial.x + i].coordinates;
        }
        number_of_exit_points += find_exit_points_on_vector(rank, nodes, size, vector1a, vector1b, chunk_side_length, exit_points, 0);
    } else if (DEBUG) {
        printf_debug("Out of top\n");
    }

    if ((initial.y + chunk_side_length) < size) {
        printf_debug("Ok bottom\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + chunk_side_length - 1) * size + initial.x + i].coordinates;
            vector2b[i] = nodes[(initial.y + chunk_side_length) * size + initial.x + i].coordinates;
        }
        number_of_exit_points += find_exit_points_on_vector(rank, nodes, size, vector2a, vector2b, chunk_side_length, exit_points, 1);
    } else if (DEBUG) {
        printf_debug("Out of bottom\n");
    }

    // Iterating left and right
    // Debug
    if ((initial.x - 1) >= 0) {
        printf_debug("Ok left\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[(initial.y + i) * size + initial.x].coordinates;
            vector1b[i] = nodes[(initial.y + i) * size + initial.x - 1].coordinates;
        }
        number_of_exit_points += find_exit_points_on_vector(rank, nodes, size, vector1a, vector1b, chunk_side_length, exit_points, 2);
    } else if (DEBUG) {
        printf_debug("Out of left\n");
    }
    if ((initial.x + chunk_side_length) < size) {
        printf_debug("Ok right\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + i) * size + initial.x + chunk_side_length - 1].coordinates;
            vector2b[i] = nodes[(initial.y + i) * size + initial.x + chunk_side_length].coordinates;
        }
        number_of_exit_points += find_exit_points_on_vector(rank, nodes, size, vector2a, vector2b, chunk_side_length, exit_points, 3);
    } else if (DEBUG) {
        printf_debug("Out of right\n");
    }

    return number_of_exit_points;
}

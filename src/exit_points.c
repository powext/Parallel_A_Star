#include <tic.h>
#include "../include/print.h"

extern int GRID_HEIGHT;
extern int GRID_WIDTH;
extern int DEBUG;

void find_chunk_corners_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    enum Direction {
        up, down, left, right
    };
    struct Angle {
        Coordinates coordinates;
        enum Direction directions[2];
    };
    struct Angle corners[4] = {
            {initial, {up, left}},
            {initial.x + chunk_side_length -1, initial.y, {right, up}},
            {initial.x + chunk_side_length -1, initial.y + chunk_side_length -1, {down, right}},
            {initial.x, initial.y + chunk_side_length -1, {left, down}}
    };
    for (int i = 0; i < 4; i++) {
        Coordinates corner = corners[i].coordinates;
        enum Direction* directions = corners[i].directions;
        if (DEBUG) {
            printf("[DEBUG] Corner: %d:%d\n", corner.x, corner.y);
        }
        for (int j = 0; j < 2; j++) {
            enum Direction direction = directions[j];
            if (DEBUG) {
                printf("[DEBUG] Direction: %d\n", direction);
            }
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
            if (DEBUG) {
                printf("[DEBUG] Exit point: %d:%d\n", outlet_coords.x, outlet_coords.y);
            }
            if (outlet_coords.x < 0 || outlet_coords.x >= GRID_WIDTH || outlet_coords.y < 0 || outlet_coords.y >= GRID_HEIGHT) {
                if (DEBUG) {
                    printf("[DEBUG] Exit point out of bounds\n");
                }
                continue;
            }
            Node* outlet_node = &nodes[outlet_coords.y * GRID_WIDTH + outlet_coords.x];
            if (outlet_node->type == obstacle) {
                if (DEBUG) {
                    printf("[DEBUG] Exit point is an obstacle\n");
                }
                continue;
            }
            Node* exit_point = &nodes[corner.y * GRID_WIDTH + corner.x];
            exit_points[i] = exit_point->coordinates;
            if (DEBUG) {
                printf("[DEBUG] Exit point is valid\n");
            }
        }
    }
}

void find_exit_points_on_vector(Node* nodes, Coordinates *vector, Coordinates* outer_vector, int vector_length, Coordinates *exit_points, int direction) {
    // debug vectors
    if (DEBUG) {
        printf("[DEBUG] R0 - vector: ");
        for (int i = 0; i < vector_length; i++) {
            printf("%d:%d ", vector[i].x, vector[i].y);
        }
        printf("\n");
        printf("[DEBUG] R0 - outer_vector: ");
        for (int i = 0; i < vector_length; i++) {
            printf("%d:%d ", outer_vector[i].x, outer_vector[i].y);
        }
        printf("\n");
    }
    int N = (N_EXIT_POINTS_PER_CHUNK - 4) / 4;
    int step = (vector_length - 2) / N; // Exclude the first and last entry

    for (int i = 1; i <= N; i++) {
        int index = 1 + (i * step) - (step / 2); // Start from the middle of the step, +1 to skip the first entry

        // Check the middle point first
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
            && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            exit_points[4 + (direction * N) + (i - 1)] = vector[index];
            continue;
        }

        // Searching to the left from the middle
        int leftIndex = index - 1;
        while (leftIndex > 1 && nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type == obstacle) { // > 1 to avoid the first entry
            leftIndex--;
        }

        // If a valid point is found to the left, add to exit_points
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
            && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            exit_points[4 + ((direction - 1) * (N - 1)) + i] = vector[leftIndex];
            continue;
        }

        // If not found in the middle or to the left, searching to the right
        int rightIndex = index + 1;
        while (rightIndex < vector_length - 1
               && (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type == obstacle
                   || nodes[vector->y * GRID_WIDTH + vector->x].type == obstacle)) {
            rightIndex++;
        }
        // If a valid point is found to the right, add to exit_points
        if (nodes[outer_vector->y * GRID_WIDTH + outer_vector->x].type != obstacle
            && nodes[vector->y * GRID_WIDTH + vector->x].type != obstacle) {
            exit_points[4 + ((direction - 1) * (N - 1)) + i] = vector[rightIndex];
        }
    }
}

void find_chunk_sides_exit_points(Node* nodes, int chunk_side_length, Coordinates initial, Coordinates *exit_points) {
    Coordinates vector1a[chunk_side_length];
    Coordinates vector1b[chunk_side_length];
    Coordinates vector2a[chunk_side_length];
    Coordinates vector2b[chunk_side_length];
    // Debug
    if (DEBUG) {
        printf("[DEBUG] R0 - initial: %d:%d\n", initial.x, initial.y);
    }
    // Iterating top and bottom
    if ((initial.y - 1) >= 0) {
        printf("[DEBUG] R0 - ok top\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[initial.y * GRID_WIDTH + initial.x + i].coordinates;
            vector1b[i] = nodes[(initial.y - 1) * GRID_WIDTH + initial.x + i].coordinates;
        }
        find_exit_points_on_vector(nodes, vector1a, vector1b, chunk_side_length, exit_points, 0);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of top\n");
    }

    if ((initial.y + chunk_side_length) < GRID_HEIGHT) {
        printf("[DEBUG] R0 - ok bottom\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + chunk_side_length - 1) * GRID_WIDTH + initial.x + i].coordinates;
            vector2b[i] = nodes[(initial.y + chunk_side_length) * GRID_WIDTH + initial.x + i].coordinates;
        }
        find_exit_points_on_vector(nodes, vector2a, vector2b, chunk_side_length, exit_points, 1);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of bottom\n");
    }

    // Iterating left and right
    // Debug
    if ((initial.x - 1) >= 0) {
        printf("[DEBUG] R0 - ok left\n");
        for (int i=0; i < chunk_side_length; i++) {
            vector1a[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x].coordinates;
            vector1b[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x - 1].coordinates;
        }
        find_exit_points_on_vector(nodes, vector1a, vector1b, chunk_side_length, exit_points, 2);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of left\n");
    }
    if ((initial.x + chunk_side_length) < GRID_WIDTH) {
        printf("[DEBUG] R0 - ok right\n");
        for (int i = 0; i < chunk_side_length; i++) {
            vector2a[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x + chunk_side_length - 1].coordinates;
            vector2b[i] = nodes[(initial.y + i) * GRID_WIDTH + initial.x + chunk_side_length].coordinates;
        }
        find_exit_points_on_vector(nodes, vector2a, vector2b, chunk_side_length, exit_points, 3);
    } else if (DEBUG) {
        printf("[DEBUG] R0 - out of right\n");
    }
}

//
// Created by Simone Bianchin on 03/02/23.
//

#include "../include/input_generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GREY  "\x1B[38;5;236m"
#define RED   "\x1B[38;5;160m"
#define GREEN "\x1B[38;5;40m"
#define WHITE "\x1B[38;5;15m"
#define RESET "\x1B[0m"

#define MAX_OBSTACLES ((HEIGHT * WIDTH) / 3)

extern int HEIGHT;
extern int WIDTH;

void init_matrix(char (*matrix)[HEIGHT]) {
    int i, j;
    for (i = 0; i < HEIGHT; i++) {
        for (j = 0; j < WIDTH; j++) {
            matrix[i][j] = '+';
        }
    }
}

void set_start(char (*matrix)[HEIGHT], int x, int y) {
    matrix[y][x] = 'S';
}

void set_end(char (*matrix)[HEIGHT], int x, int y) {
    matrix[y][x] = 'E';
}

void set_obstacle(char (*matrix)[HEIGHT], int x, int y) {
    matrix[y][x] = '-';
}

void print_with_colors(char c) {
    if (c == '+')
        printf(GREY "%c" RESET, c);
    else if (c == 'S')
        printf(GREEN "%c" RESET, c);
    else if (c == 'E')
        printf(RED "%c" RESET, c);
    else if (c == '-')
        printf(WHITE "%c" RESET, c);
}

void generate_input(char (*matrix)[HEIGHT]) {
    int i;
    srand(time(NULL));

    init_matrix(matrix);

    int start_x = rand() % WIDTH;
    int start_y = rand() % HEIGHT;

    int end_x = rand() % WIDTH;
    int end_y = rand() % HEIGHT;

    set_start(matrix, start_x, start_y);
    set_end(matrix, end_x, end_y);

    int obstacle_count = 0;
    while (obstacle_count < MAX_OBSTACLES) {
        int obstacle_x = rand() % WIDTH;
        int obstacle_y = rand() % HEIGHT;

        if (matrix[obstacle_y][obstacle_x] == '+') {
            set_obstacle(matrix, obstacle_x, obstacle_y);
            obstacle_count++;
        }
    }

    for (i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            print_with_colors(matrix[i][j]);
        }
        printf("\n");
    }

    return;
}

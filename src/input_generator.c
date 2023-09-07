//
// Created by Simone Bianchin on 03/02/23.
//

#include "../include/input_generator.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "../include/comm.h"
#include "../include/compute_path.h"

#define GREY  "\x1B[38;5;236m"
#define RED   "\x1B[38;5;160m"
#define GREEN "\x1B[38;5;40m"
#define WHITE "\x1B[38;5;15m"
#define RESET "\x1B[0m"


void init_matrix(char** matrix, int size) {
    int i, j;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            matrix[i][j] = '+';
        }
    }
}

void set_start(char** matrix, int x, int y) {
    matrix[y][x] = 'S';
}

void set_end(char** matrix, int x, int y) {
    matrix[y][x] = 'E';
}

void set_obstacle(char** matrix, int x, int y) {
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

double compute_heuristic_tmp(Coordinates a, Coordinates b){
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    return (double) (dx + dy);
}

NodeType get_type(char node){
    if (node == '+')
        return cell;
    else if (node == 'S')
        return start;
    else if (node == 'E')
        return end;
    else if (node == '-')
        return  obstacle;
}

bool test_matrix(char** matrix, int size, int start_x, int start_y, int end_x, int end_y){
    Coordinates start = {start_x, start_y};
    Coordinates end = {end_x, end_y};

    Node** tmp_matrix = malloc(size*sizeof(Node*));
    for(int i = 0; i < size; i++){
        tmp_matrix[i] = malloc(size*sizeof(Node));
    }

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            Node* tmp = malloc(sizeof(Node));
            tmp->id = i*size+j;
            tmp->type = get_type(matrix[i][j]);
            tmp->coordinates.x = i;
            tmp->coordinates.y = j;
            tmp->distance = INT16_MAX-2;
            tmp->heuristic = compute_heuristic_tmp(tmp->coordinates, end);
            tmp_matrix[j][i] = *tmp;
        }
    }

    ChunkPath* path = compute_path(tmp_matrix, size, size, start, end);
    if (path->n_nodes > 0){
        printf("Path found!\n");
        return true;
    }

    printf("Path empty!\n");
    return false;
}

void write_matrix(char** matrix, int size){
    printf("Writing file!\n");
    char dimension[9];
    char filename[50];
    sprintf(dimension, "%d", size);
    printf("Getting filename!\n");
    sprintf(filename, "data/matrix_%s.txt", dimension);

    printf("Opening file!\n");
    FILE *file = fopen(filename, "w");

    if (file == NULL) {
        perror("Error opening file");
        exit(1); // Exit with an error code
    }

    printf("Writing content!\n");
    // Write the content of the matrix to the file
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            fprintf(file, "%c", matrix[i][j]);
        }
        fprintf(file, "\n"); // Move to the next line
    }

    // Close the file
    fclose(file);
    printf("Wrote file!\n");
}

void generate_input(int size) {
    int i;
    srand(time(NULL));
    int max_obstacle = size*size / 3;
    char** matrix = malloc(size*sizeof(char*));
    for(int a = 0; a < size; a++){
        matrix[a] = malloc(size* sizeof(char));
    }
    
    init_matrix(matrix, size);

    int start_x = rand() % size;
    int start_y = rand() % size;

    int end_x = rand() % size;
    int end_y = rand() % size;

    set_start(matrix, start_x, start_y);
    set_end(matrix, end_x, end_y);

    int obstacle_count = 0;
    while (obstacle_count < max_obstacle) {
        int obstacle_x = rand() % size;
        int obstacle_y = rand() % size;

        if (matrix[obstacle_y][obstacle_x] == '+') {
            set_obstacle(matrix, obstacle_x, obstacle_y);
            obstacle_count++;
        }
    }

    /*for (i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            print_with_colors(matrix[i][j]);
        }
        printf("\n");
    }*/

    if(test_matrix(matrix, size, start_x, start_y, end_x, end_y)){
        write_matrix(matrix, size);
    } else{
        for(int a = 0; a < size; a++){
            free(matrix[a]);
        }
        free(matrix);
        generate_input(size);
    }
}

int main(int argc, char** argv){
    int dimensions = 5000;
    generate_input(dimensions);
    return 0;
}
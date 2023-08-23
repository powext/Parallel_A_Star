#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "../include/compute_path.h"
#include "../include/compute_distance.h"

int HEIGHT = 30;
int WIDTH = 30;

void initialize_nodes_from_matrix(char input_matrix[HEIGHT][WIDTH], Node* nodes, Node** starting_node, Node** destination_node) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            char curr = input_matrix[i][j];
            int id = (HEIGHT * i) + j;
            nodes[id] = (Node){
                    .id = id,
                    .coordinates.x = j,
                    .coordinates.y = i,
            };

            if (curr == '-')
                nodes[id].type = obstacle;
            else if (curr == 'S') {
                nodes[id].type = start;
                *starting_node = &nodes[id];
            }
            else if (curr == 'E') {
                nodes[id].type = end;
                *destination_node = &nodes[id];
            }
            else if (curr == '+')
                nodes[id].type = cell;
        }
    }
}

void initialize_nodes_from_file(int size, Node* nodes, Node** starting_node, Node** destination_node) {
    char filename[100];
    sprintf(filename, "data/matrix_%d.txt", size);
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("[ERROR] Opening %s!\n", filename);
        exit(1);
    }

    char curr;
    int i = 0;
    int j = 0;
    while ((curr = fgetc(fp)) != EOF) {
        if (curr == '\n') {
            i++;
            j = 0;
            continue;
        }
        int id = (HEIGHT * i) + j;
        nodes[id] = (Node){
                .id = id,
                .coordinates.x = j,
                .coordinates.y = i,
        };

        if (curr == '-')
            nodes[id].type = obstacle;
        else if (curr == 'S') {
            nodes[id].type = start;
            *starting_node = &nodes[id];

        }
        else if (curr == 'E') {
            nodes[id].type = end;
            *destination_node = &nodes[id];
        }
        else if (curr == '+')
            nodes[id].type = cell;
        j++;
    }
}

char* look_for_file(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-file") == 0) {
                // The next argument (i + 1) is the value for the "-input" parameter
                if (i + 1 < argc) {
                    printf("Input file: %s\n", argv[i + 1]);
                    return argv[i + 1];
                } else {
                    printf("Missing value for -input parameter.\n");
                }
            }
        }
    }
    return NULL;
}

int look_for_mode(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-parallel") == 0) {
                // The next argument (i + 1) is the value for the "-output" parameter
                return 1;
            }
        }
    }

    return 0;
}

int look_for_size(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-size") == 0) {
                // The next argument (i + 1) is the value for the "-input" parameter
                if (i + 1 < argc) {
                    printf("Maze dimension: %s\n", argv[i + 1]);
                    return (int) strtol(argv[i + 1], NULL, 10); // Skip the value argument
                } else {
                    printf("Missing value for -size parameter.\n");
                }
            }
        }
    }

    return 0;
}

int find_maze_size(char* filename){
    // Open the file
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        perror("Error opening the file");
        return 1;
    }

    // Calculate the length of the first row without whitespaces
    int length = 0;
    int c; // Variable to hold the current character

    while ((c = fgetc(file)) != EOF && c != '\n') {
        if (!isspace(c)) {
            length++;
        }
    }

    // Close the file
    fclose(file);

    return length;
}

bool is_in_path(Node* node, Coordinates **pCoordinates, int n_node) {
    for (int i = 0; i < n_node; i++){
        if (node->coordinates.x == pCoordinates[i]->x && node->coordinates.y == pCoordinates[i]->y){
            return true;
        }
    }
    return false;
}

void print_node(Node node, bool in_path){
    if (in_path){
        printf("\x1B[38;5;40m");
    }

    if (node.type == 3){
        printf("\x1B[38;5;236m-\x1B[0m");
    } else if (node.type == 2){
        printf("+\x1B[0m");
    } else if (node.type == 0){
        printf("S\x1B[0m");
    } else {
        printf("E\x1B[0m");
    }

}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);

    Node* starting_node;
    Node* destination_node;
    Node* nodes;

    // file parameter imports data from the data/ directory
    char* filename = look_for_file(argv, argc);
    if (filename != NULL) {
        int matrix_input_size = look_for_size(argv, argc);
        if (matrix_input_size < 1){
            matrix_input_size = find_maze_size(filename);
            if (matrix_input_size < 1){
                printf("Maze dimension not specified! Please use -size parameter");
                exit(1);
            }
        }
        HEIGHT = WIDTH = matrix_input_size;
        printf("[INFO] Taking data from matrix_%d.txt\n", matrix_input_size);
        nodes = malloc(matrix_input_size * matrix_input_size * sizeof(Node));
        initialize_nodes_from_file(matrix_input_size, nodes, &starting_node, &destination_node);
    } else {
        printf("[INFO] Input to be generated\n");
        int matrix_input_size = look_for_size(argv, argc);
        if (matrix_input_size < 1){
            printf("Matrix dimension not specified! Please use -size parameter");
            exit(1);
        }
        HEIGHT = WIDTH = matrix_input_size;
        char input_matrix[HEIGHT][WIDTH];
        // generate_input(input_matrix);
        nodes = malloc(HEIGHT * WIDTH * sizeof(Node));
        initialize_nodes_from_matrix(input_matrix, nodes, &starting_node, &destination_node);
    }

    // parallel parameter runs the parallel version of the program instead of the serial one
    if (look_for_mode(argv, argc)) {
        printf("[INFO] Algorithm running in parallel configuration\n");
        // parallel_root_init(nodes);
        // parallel_finalize();
    } else {
        printf("[INFO] Algorithm running in serial configuration\n");
        printf("[INFO] Creating matrix\n");
        Node** matrix = (Node**)malloc(sizeof(Node*) * HEIGHT);
        for (int i = 0; i < HEIGHT; i++) {
            matrix[i] = &nodes[i * WIDTH];
        }
        for (int i=0; i<WIDTH; i++){
            for(int j=0; j<HEIGHT; j++){
                matrix[i][j].distance = INT16_MAX/2;
                matrix[i][j].heuristic_distance = compute_heuristic(matrix[i][j], *destination_node);
            }
        }

        printf("[INFO] Searching for path\n");
        ChunkPath* tmp = compute_path(matrix, WIDTH, HEIGHT, starting_node->coordinates, destination_node->coordinates);

        printf("[INFO] Path found\n");

        printf("[INFO] Printing path\n");
        for (int i=0; i<WIDTH; i++){
            for (int j = 0; j < HEIGHT; j++){
                print_node(matrix[i][j], is_in_path(&matrix[i][j], tmp->nodes, tmp->n_nodes));
            }
            printf("\n");
        }
    }
    free(nodes);
    return 0;
}


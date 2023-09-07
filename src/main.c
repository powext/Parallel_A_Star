#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "../include/comm.h"
#include "../include/compute_path.h"
#include "../include/compute_distance.h"
#include "../include/parallel.h"
#include "../include/parallel_distribution.h"

int DEBUG = 0;

void initialize_nodes_from_file(char* file, int size, Node* nodes, Node** starting_node, Node** destination_node) {

    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        printf("[ERROR] Opening %s!\n", file);
        exit(1);
    }

    char curr;
    int i = 0;
    int j = 0;
    while ((curr = fgetc(fp)) != EOF) {
        if (curr == '\n') {
            i++;
            if (j < size) {
                printf("[ERROR] Matrix size specified GRID_WIDTH is wider than the file rows!\n");
                exit(1);
            }
            j = 0;
            continue;
        }
        int id = (size * i) + j;
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

//    for(int a = 0; a < size*size; a++)
//        printf("%d\t", nodes[a].id);
}

char* look_for_file(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-file") == 0) {
                // The next argument (i + 1) is the value for the "-input" parameter
                if (i + 1 < argc) {
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
                return 1;
            }
        }
    }
    return 0;
}

int check_debug(int argc, char** argv) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-debug") == 0) {
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

bool is_in_path(Node* node, Coordinates* pCoordinates, int n_node) {
    for (int i = 0; i < n_node; i++){
        if (node->coordinates.x == pCoordinates[i].x && node->coordinates.y == pCoordinates[i].y){
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

void print_matrix(Node** matrix, int size, ChunkPath* path){
    for (int i=0; i<size; i++){
        for (int j = 0; j < size; j++){
            print_node(matrix[i][j], is_in_path(&matrix[i][j], path->nodes, path->n_nodes));
        }
        printf("\n");
    }
}

int get_matrix_size(int argc, char** argv, char* filename){
    int matrix_input_size = look_for_size(argv, argc);
    if (matrix_input_size < 1){
        matrix_input_size = find_maze_size(filename);
        if (matrix_input_size < 1){
            printf("[ERROR] Maze dimension not specified! Please use -size parameter");
        }
    }
    return matrix_input_size;
}

void initialise_matrix_distances(Node** matrix, int size, Node* destination_node){
    for (int i=0; i<size; i++){
        for(int j=0; j<size; j++){
            matrix[i][j].distance = INT16_MAX / 2;
            matrix[i][j].heuristic = compute_heuristic(matrix[i][j], *destination_node);
            matrix[i][j].score = matrix[i][j].distance + matrix[i][j].heuristic;
        }
    }
}

void initialise_node_list_distances(Node* nodes, int size, Node* destination_node){
    for (int i=0; i<size; i++){
            nodes[i].distance = INT16_MAX / 2;
            nodes[i].heuristic = compute_heuristic(nodes[i], *destination_node);
            nodes[i].score = nodes[i].distance + nodes[i].heuristic;
    }
}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);

    Node* starting_node;
    Node* destination_node;
    Node* nodes;
    int matrix_input_size;
    double current_time;

    DEBUG = check_debug(argc, argv);
    // parallel parameter runs the parallel version of the program instead of the serial one
    if (look_for_mode(argv, argc)) {
        int* n_chunks = malloc(sizeof(int));
        int* world_rank = malloc(sizeof(int));

        parallel_init(n_chunks, world_rank);
        printf("[INFO][PROCESS %d] Algorithm running in parallel configuration\n", *world_rank);
        if (*world_rank == 0){

            // file parameter imports data from the data/ directory
            if(DEBUG)
                printf("[DEBUG][PROCESS %d] Searching for file!\n", *world_rank);
            char* filename = look_for_file(argv, argc);

            if(DEBUG)
                printf("[DEBUG][PROCESS %d] Filename: %s\n", *world_rank, filename);

            if (filename != NULL) {
                matrix_input_size = get_matrix_size(argc, argv, filename);
                if (DEBUG)
                    printf("[DEBUG][PROCESS %d] Taking data from %s!\n", *world_rank, filename);
                nodes = malloc(matrix_input_size * matrix_input_size * sizeof(Node));
                initialize_nodes_from_file(filename, matrix_input_size, nodes, &starting_node, &destination_node);
            } else {
                printf("Please specify a maze to resolve using -file option!");
                parallel_finalize();
                exit(1);
            }

            initialise_node_list_distances(nodes, matrix_input_size*matrix_input_size, destination_node);
            if (DEBUG)
                printf("[DEBUG][PROCESS %d] Initialised matrix of dimension %d!\n", *world_rank, matrix_input_size);

        }

        MPI_Barrier(MPI_COMM_WORLD);

        if(*world_rank == 0){
            for(int i = 1; i < *n_chunks; i++){
                MPI_Send(&matrix_input_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        } else {
            MPI_Recv(&matrix_input_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if(*world_rank == 0){
            if (DEBUG)
                printf("[DEBUG][PROCESS %d] Getting time!\n", *world_rank);
            current_time = MPI_Wtime();
        }
        MPI_Barrier(MPI_COMM_WORLD);
        distribute_work(nodes, matrix_input_size, starting_node, destination_node, *n_chunks, *world_rank);
        // parallel_compute_paths();
        MPI_Barrier(MPI_COMM_WORLD);
        if(*world_rank == 0){
            current_time = MPI_Wtime()-current_time;
        }
        MPI_Barrier(MPI_COMM_WORLD);
        parallel_finalize();
        printf("Time: %2f", current_time);
    } else {
        printf("[INFO] Algorithm running in serial configuration\n");

        // file parameter imports data from the data/ directory
        char* filename = look_for_file(argv, argc);
        if (filename != NULL) {
            matrix_input_size = get_matrix_size(argc, argv, filename);

            printf("[INFO] Taking data from %s\n", filename);
            nodes = malloc(matrix_input_size * matrix_input_size * sizeof(Node));
            initialize_nodes_from_file(filename, matrix_input_size, nodes, &starting_node, &destination_node);
        } else {
            printf("Please specify a maze to resolve using -file option!");
            exit(1);
        }

        Node** matrix = (Node**)malloc(sizeof(Node*) * matrix_input_size);
        for (int i = 0; i < matrix_input_size; i++) {
            matrix[i] = &nodes[i * matrix_input_size];
        }

        initialise_matrix_distances(matrix, matrix_input_size, destination_node);

        clock_t start_time = clock();
        printf("[INFO] Searching for path\n");
        ChunkPath* tmp = compute_path(matrix, matrix_input_size, matrix_input_size, starting_node->coordinates, destination_node->coordinates);

        if(tmp->n_nodes > 0){
            clock_t end_time = clock();
            current_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
            printf("[INFO] Path found in \n");
            printf("%d, %2f, s\n", matrix_input_size, current_time);
            // print_matrix(matrix, matrix_input_size, tmp);
        } else {
            printf("[INFO] Path not found\n");
            printf("%d, NA, s\n", matrix_input_size);
        }

    }

    return 0;
}


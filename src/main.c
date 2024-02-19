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
#include "../include/utility.h"
#include "../include/json_output.h"

bool DEBUG = false;
int N_EXIT_POINTS_PER_CHUNK = 12;

void initialize_nodes_from_file(char* file, int size, Node* nodes, Node** starting_node, Node** destination_node) {
    FILE* fp = fopen(file, "r");
    if (fp == NULL) {
        printf_debug("[ERROR] Opening %s!\n", file);
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
}

char* look_for_file(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-file") == 0) {
                // The next argument (i + 1) is the value for the "-file" parameter
                if (i + 1 < argc) {
                    return argv[i + 1];
                } else {
                    printf_debug("Missing value for -file parameter.\n");
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

bool check_debug(int argc, char** argv) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-debug") == 0) {
                return true;
            }
        }
    }
    return false;
}

int look_for_size(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-size") == 0) {
                // The next argument (i + 1) is the value for the "-size" parameter
                if (i + 1 < argc) {
                    return (int) strtol(argv[i + 1], NULL, 10); // Skip the value argument
                } else {
                    printf_debug("Missing value for -size parameter.\n");
                }
            }
        }
    }

    return 0;
}

int look_for_n_exitpoints(char** argv, int argc) {
    for (int i = 1; i < argc; i++) { // Start from 1 to skip the program name (argv[0])
        // Check if the argument is a named parameter (starts with '-')
        if (argv[i][0] == '-') {
            // Compare the argument with various named parameters
            if (strcmp(argv[i], "-exitpoints") == 0) {
                // The next argument (i + 1) is the value for the "-size" parameter
                if (i + 1 < argc) {
                    return (int) strtol(argv[i + 1], NULL, 10); // Skip the value argument
                }
            }
        }
    }

    return N_EXIT_POINTS_PER_CHUNK;
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

void print_matrix(Node* matrix, int size, ChunkPath* path){
    for (int i=0; i<size; i++){
        for (int j = 0; j < size; j++){
            print_node(matrix[i*size + j], is_in_path(&matrix[i*size + j], path->nodes, path->n_nodes));
        }
        printf("\n");
    }
}

int get_matrix_size(int argc, char** argv, char* filename){
    int matrix_input_size = look_for_size(argv, argc);
    if (matrix_input_size < 1){
        matrix_input_size = find_maze_size(filename);
        if (matrix_input_size < 1){
            printf_debug("[ERROR] Maze dimension not specified! Please use -size parameter");
        }
    }
    return matrix_input_size;
}


void initialise_node_list_distances(Node* nodes, int size, Node* destination_node){
    for (int i=0; i<size; i++){
            nodes[i].distance = INT16_MAX / 2;
            nodes[i].heuristic = compute_heuristic_nodes(&nodes[i], destination_node);
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
    clock_t step_time_start;
    clock_t step_time_end;

    DEBUG = check_debug(argc, argv);

    // parallel parameter runs the parallel version of the program instead of the serial one
    if (look_for_mode(argv, argc)) {
        int* n_chunks = malloc(sizeof(int));
        int* world_rank = malloc(sizeof(int));

        parallel_init(n_chunks, world_rank);
        printf_debug("Algorithm running in parallel configuration\n");

        N_EXIT_POINTS_PER_CHUNK = look_for_n_exitpoints(argv, argc);
        printf_debug("N_EXIT_POINTS_PER_CHUNK: %d\n", N_EXIT_POINTS_PER_CHUNK);

        if (*world_rank == 0) {
            // file parameter imports data from the data/ directory
            step_time_start = clock();
            if(DEBUG)
                printf_debug(" Searching for file!\n");
            char* filename = look_for_file(argv, argc);

            if(DEBUG)
                printf_debug(" Filename: %s\n", filename);

            if (filename != NULL) {
                matrix_input_size = get_matrix_size(argc, argv, filename);
                if (DEBUG)
                    printf_debug(" Taking data from %s!\n", filename);
                nodes = malloc(matrix_input_size * matrix_input_size * sizeof(Node));
                initialize_nodes_from_file(filename, matrix_input_size, nodes, &starting_node, &destination_node);
            } else {
                printf_debug("Please specify a maze to resolve using -file option!\n");
                parallel_finalize();
                exit(1);
            }

            initialise_node_list_distances(nodes, matrix_input_size*matrix_input_size, destination_node);
            if (DEBUG)
                printf_debug("Initialised matrix of dimension %d!\n", matrix_input_size);

        }

        MPI_Barrier(MPI_COMM_WORLD);
        if (*world_rank == 0) {
            step_time_end = clock();
            double diff_time_initialization = (double)(step_time_end - step_time_start) / CLOCKS_PER_SEC;
            printf("Initialization took: %2f seconds\n", diff_time_initialization);
            step_time_start = clock();
        }

        if(*world_rank == 0){
            for(int i = 1; i < *n_chunks; i++){
                MPI_Send(&matrix_input_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            }
        } else {
            MPI_Recv(&matrix_input_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Barrier(MPI_COMM_WORLD);

        // TODO: measure execution time for single process then average? (see slide lectures)
        if(*world_rank == 0){
            printf_debug("Getting time!\n");
            current_time = MPI_Wtime();
        }
        MPI_Barrier(MPI_COMM_WORLD);

        MsgChunkStart** start_msgs = malloc(sizeof(MsgChunkStart*));
        AdjList** graph = malloc(sizeof(AdjList*));
        MsgChunkEnd* receivedMsgs = distribute_work(nodes, graph, start_msgs, matrix_input_size, starting_node, destination_node, *n_chunks, *world_rank, &step_time_start);

        if (*world_rank != 0) {
            parallel_finalize();
            exit(0);
        }

        // print AdjList
        if(DEBUG) {
            printf_debug("AdjList:\n");
            for (int i = 0; i < matrix_input_size * matrix_input_size; i++) {
                printf_debug("Node %d: ", i);
                Edge *edge = get_index_iterator(*graph, i);
                while (edge) {
                    printf("%d ", edge->node->id);
                    edge = edge->next;
                }
                printf_debug("\n");
            }
        }

        ChunkPath* final_path = compute_path(
                nodes,
                *graph,
                matrix_input_size,
                matrix_input_size,
                starting_node->coordinates,
                destination_node->coordinates,
                compute_weight_edges,
                compute_heuristic_nodes,
                get_neighbours_edges,
                reassemble_final_path_edges
        );

        if (*world_rank == 0) {
            step_time_end = clock();
            double diff_time_final_computation = (double)(step_time_end - step_time_start) / CLOCKS_PER_SEC;
            printf("Final computation took: %2f seconds\n", diff_time_final_computation);
            step_time_start = clock();
        }

        if (DEBUG) {
            printf("Final path (total nodes: %d):\n", final_path->n_nodes);
            for (int i = 0; i < final_path->n_nodes; i++) {
                printf("Node %d: %d:%d\n", i, final_path->nodes[i].x, final_path->nodes[i].y);
            }
        }
        free_graph(*graph, matrix_input_size*matrix_input_size);

        output_json(nodes, matrix_input_size, starting_node, destination_node, *start_msgs, receivedMsgs, final_path, *world_rank, *n_chunks);

        for (int i = 0; i < *n_chunks; i++) {
            for (int j = 0; j < receivedMsgs[i].num_of_paths; j++) {
                free(receivedMsgs[i].paths[j].nodes);
                free(receivedMsgs[i].paths[j].exit_points);
            }
            free(receivedMsgs[i].paths);
        }
        free(receivedMsgs);
        free(start_msgs);
        free(nodes);

        current_time = MPI_Wtime()-current_time;
        parallel_finalize();
        printf("%d, %2f, s\n", matrix_input_size, current_time);
        if(final_path->n_nodes <= 0){
            printf("Path not found\n");
        }
    } else {
        printf_debug("[INFO] Algorithm running in serial configuration\n");

        // file parameter imports data from the data/ directory
        char* filename = look_for_file(argv, argc);
        if (filename != NULL) {
            matrix_input_size = get_matrix_size(argc, argv, filename);

            printf_debug("[INFO] Taking data from %s\n", filename);
            nodes = malloc(matrix_input_size * matrix_input_size * sizeof(Node));
            initialize_nodes_from_file(filename, matrix_input_size, nodes, &starting_node, &destination_node);
        } else {
            printf("Please specify a maze to resolve using -file option!\n");
            exit(1);
        }

        initialise_node_list_distances(nodes, matrix_input_size*matrix_input_size, destination_node);

        clock_t start_time = clock();
        printf_debug("Searching for path\n");
        ChunkPath* tmp = compute_path(nodes, NULL, matrix_input_size, matrix_input_size, starting_node->coordinates, destination_node->coordinates, compute_weight_nodes, compute_heuristic_nodes, get_neighbours_nodes, reassemble_final_path_nodes);

        clock_t end_time = clock();
        current_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
        printf("%d, %2f, s\n", matrix_input_size, current_time);
        if(tmp->n_nodes <= 0){
            printf_debug("Path not found\n");
        }

    }

    return 0;
}


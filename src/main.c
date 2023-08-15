#include <printf.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/input_generator.h"
#include "../include/node.h"
#include "../include/priority_queue.h"
#include "../include/generic_list.h"
#include "../include/compute_distance.h"
#include "../include/print.h"
#include "../include/parallel.h"
#include "../include/comm.h"

int HEIGHT = 30;
int WIDTH = 30;

LinkedNode* add_neighbour(LinkedNode* curr, Node* neighbour) {
    LinkedNode* linked_neighbour = malloc(sizeof(LinkedNode));
    linked_neighbour->node = neighbour;
    linked_neighbour->next = curr->next;
    curr->next = linked_neighbour;
    return linked_neighbour;
}

LinkedNode* get_neighbours(List* arches_list, Node *current) {
    return ((LinkedNode *) arches_list->arr[current->id])->next;
}

void print_context(Node* nodes, List* visited_nodes) {
    for (int i = 0; i < visited_nodes->used; i++) {
        Node visited_node = *(Node*) visited_nodes->arr[i];
        Node* context_node = &nodes[visited_node.id];
        if (context_node->type == start) continue;
        // printf("%f\n", context_node->distance);
        context_node->type = visited;
    }
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            printf_with_colors(nodes[(HEIGHT * i) + j]);
        }
        printf("\n");
    }
}

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

void initialize_arches_list(Node* nodes, List* arches_list) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int id = (HEIGHT * i) + j;
            Node* curr = &nodes[id];
            LinkedNode* linked_curr = malloc(sizeof(LinkedNode));
            linked_curr->node = curr;
            linked_curr->next = NULL;
            LinkedNode* linked_succ = NULL;
            if (curr->coordinates.x - 1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(HEIGHT * curr->coordinates.y) + (curr->coordinates.x - 1)]
                );
            if (curr->coordinates.y - 1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(HEIGHT * (curr->coordinates.y - 1)) + curr->coordinates.x]
                );
            if (curr->coordinates.x + 1 < WIDTH)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(HEIGHT * curr->coordinates.y) + (curr->coordinates.x + 1)]
                );
            if (curr->coordinates.y + 1 < HEIGHT)
                add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(HEIGHT * (curr->coordinates.y + 1)) + curr->coordinates.x]
                );
            insert_into_list(arches_list, linked_curr);
        }
    }
}

void initialize_lists(Node* starting_node, Node* destination_node, MinHeap** open_list, List** close_list) {
    *open_list = init_minheap();
    *close_list = init_list();

    starting_node->normal_distance = 0;
    starting_node->heuristic_distance = compute_heuristic(*starting_node, *destination_node);
    starting_node->distance = starting_node->heuristic_distance;
    *open_list = insert_into_heap(*open_list, starting_node);
}

void process_neighbours(List* arches_list, MinHeap* open_list, List* close_list, Node* destination_node, Node* curr_node) {
    LinkedNode* linked_neighbour = get_neighbours(arches_list, curr_node);
    List* neighbours = init_list();
    int n_neighbours = 0;
    while (linked_neighbour != NULL) {
        insert_into_list(neighbours, linked_neighbour->node);
        linked_neighbour = linked_neighbour->next;
        n_neighbours++;
    }

    for (int i = 0; i < n_neighbours; i++) {
        Node* curr_neighbour = neighbours->arr[i];
        if (curr_neighbour->type == obstacle) continue;
        bool is_node_already_visited = find_in_list(close_list, curr_neighbour);
        if (is_node_already_visited) {
            continue;
        }

        curr_neighbour->heuristic_distance = compute_heuristic(*curr_neighbour, *destination_node);
        curr_neighbour->normal_distance = 0;
        curr_neighbour->distance = curr_neighbour->heuristic_distance;

        bool is_node_already_open = find_in_heap(open_list, curr_neighbour);
        if (is_node_already_open)
            continue;

        insert_into_heap(open_list, curr_neighbour);
    }

    free_list(neighbours);
}

void search_path(Node* starting_node, Node* destination_node, Node* nodes, List* arches_list) {
    MinHeap* open_list;
    List* close_list;

    initialize_lists(starting_node, destination_node, &open_list, &close_list);

    bool completed = false;
    while (!is_queue_empty(open_list) && !completed) {
        printf("----\n");
        Node* curr_node = pop_min(open_list);

        if (curr_node->id == destination_node->id) {
            // Final node reached
            print_context(nodes, close_list);
            free_list(arches_list);
            free_list(close_list);
            free_minheap(open_list);
            completed = true;
            continue;
        }
        insert_into_list(close_list, curr_node);

        process_neighbours(arches_list, open_list, close_list, destination_node, curr_node);

        print_context(nodes, close_list);
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
                    return (int) argv[i + 1]; // Skip the value argument
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
        generate_input(input_matrix);
        nodes = malloc(HEIGHT * WIDTH * sizeof(Node));
        initialize_nodes_from_matrix(input_matrix, nodes, &starting_node, &destination_node);
    }

    // parallel parameter runs the parallel version of the program instead of the serial one
    int mode = look_for_mode(argv, argc);
    if (mode) {
        printf("[INFO] Algorithm running in parallel configuration\n");
        parallel_root_init(nodes);
        parallel_finalize();
    } else {
        printf("[INFO] Algorithm running in serial configuration\n");
        List* arches_list = init_list();
        initialize_arches_list(nodes, arches_list);

        search_path(starting_node, destination_node, nodes, arches_list);
    }
    free(nodes);
    return 0;
}


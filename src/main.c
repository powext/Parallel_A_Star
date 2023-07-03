#include <printf.h>
#include "../include/input_generator.h"
#include "../include/node.h"
#include "../include/priority_queue.h"
#include "../include/generic_list.h"
#include "../include/compute_distance.h"
#include "../include/print.h"

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
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            printf_with_colors(nodes[(MAX_HEIGHT*i)+j]);
        }
        printf("\n");
    }
}

void initialize_nodes(char input_matrix[MAX_HEIGHT][MAX_WIDTH], Node* nodes, Node** starting_node, Node** destination_node) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            char curr = input_matrix[i][j];
            int id = (MAX_HEIGHT * i) + j;
            nodes[id] = (Node){
                    .id = id,
                    .x = j,
                    .y = i,
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

void initialize_arches_list(Node* nodes, List* arches_list) {
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            int id = (MAX_HEIGHT * i) + j;
            Node* curr = &nodes[id];
            LinkedNode* linked_curr = malloc(sizeof(LinkedNode));
            linked_curr->node = curr;
            linked_curr->next = NULL;
            LinkedNode* linked_succ = NULL;
            if (curr->x - 1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * curr->y) + (curr->x - 1)]
                );
            if (curr->y - 1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * (curr->y - 1)) + curr->x]
                );
            if (curr->x + 1 < MAX_WIDTH)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * curr->y) + (curr->x + 1)]
                );
            if (curr->y + 1 < MAX_HEIGHT)
                add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * (curr->y + 1)) + curr->x]
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

int main() {
    setbuf(stdout, NULL);

    char input_matrix[MAX_HEIGHT][MAX_WIDTH];
    generate_input(input_matrix);

    Node* starting_node;
    Node* destination_node;
    Node* nodes = malloc(MAX_HEIGHT * MAX_WIDTH * sizeof(Node));

    initialize_nodes(input_matrix, nodes, &starting_node, &destination_node);

    List* arches_list = init_list();
    initialize_arches_list(nodes, arches_list);

    search_path(starting_node, destination_node, nodes, arches_list);

    return -1;
}


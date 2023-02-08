#include <printf.h>
#include "../include/input_generator.h"
#include "../include/node.h"
#include "../include/priority_queue.h"
#include "../include/generic_list.h"
#include "../include/compute_distance.h"

LinkedNode* add_neighbour(LinkedNode* curr, Node* neighbour) {
    LinkedNode* linked_neighbour = malloc(sizeof(LinkedNode));
    linked_neighbour->node = neighbour;
    linked_neighbour->next = curr->next;
    curr->next = linked_neighbour;
    return linked_neighbour;
}

LinkedNode* get_neighbours(List* arches_list, Node current) {
    return ((LinkedNode *) arches_list->arr[current.id])->next;
}

int main() {
    char input_matrix[MAX_HEIGHT][MAX_WIDTH];
    generate_input(input_matrix);

    // select start and end nodes
    Node starting_node;
    Node destination_node;
    Node* nodes = malloc(MAX_HEIGHT * MAX_WIDTH * sizeof (Node));
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            char curr = input_matrix[i][j];
            int id = (MAX_HEIGHT*i)+j;
            nodes[id] = (Node) {
                .id =  id,
                .x =  j,
                .y =  i,
            };

            if (curr == '-')
                nodes[id].type = obstacle;
            else if (curr == 'S') {
                nodes[id].type = start;
                starting_node = nodes[id];
            }
            else if (curr == 'E') {
                nodes[id].type = end;
                destination_node = nodes[id];
            }
            else if (curr == '+')
                nodes[id].type = cell;
        }
    }

    List* arches_list = init_list();
    for (int i = 0; i < MAX_HEIGHT; i++) {
        for (int j = 0; j < MAX_WIDTH; j++) {
            int id = (MAX_HEIGHT*i)+j;
            Node* curr = &nodes[id];
            LinkedNode* linked_curr = malloc(sizeof (LinkedNode));
            linked_curr->node = curr;
            LinkedNode* linked_succ = NULL;
            linked_curr->next = linked_succ;
            if (curr->x -1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * curr->y) + (curr->x -1)]
                );
            if (curr->y -1 >= 0)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * (curr->y -1)) + curr->x]
                );
            if (curr->x +1 < MAX_WIDTH)
                linked_succ = add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * curr->y) + (curr->x +1)]
                );
            if (curr->y +1 < MAX_HEIGHT)
                add_neighbour(
                        linked_succ == NULL ? linked_curr : linked_succ,
                        &nodes[(MAX_HEIGHT * (curr->y +1)) + curr->x]
                );
            insert_into_list(arches_list, linked_curr);
        }
    }

    setbuf(stdout, NULL);

    for (int i = 0; i < arches_list->used; i++) {
        LinkedNode* curr = (LinkedNode*) arches_list->arr[i];
        printf("\n%d: ", curr->node->id);
        curr = curr->next;
        while (curr != NULL) {
            printf("%d ", curr->node->id);
            LinkedNode* to_free = curr;
            curr = curr->next;
            free(to_free);
        }
    }
    free_list(arches_list);

    // todo: read input graph -> read_data.c
    // read graph
    // Node* graph;

    // initialize list for node seen and to see
    MinHeap* open_list = init_minheap(0);
    List* close_list = init_list();

    // initialize starting node
    starting_node.normal_distance = 0;
    starting_node.heuristic_distance = compute_heuristic(starting_node, destination_node);
    starting_node.distance = starting_node.normal_distance;
    open_list = insert_into_heap(open_list, starting_node);


    while (!is_queue_empty(open_list)) {
        // get first element in queue
        Node current_node = pop_min(open_list);

        if (current_node.id == destination_node.id){
            // print_list(close_list); TODO
            printf("Done?");
            return 0;
        }

        // get neighbouring nodes
        LinkedNode* linked_neighbour = get_neighbours(arches_list, current_node);
        List* neighbours = init_list();
        while(linked_neighbour != NULL) {
            insert_into_list(neighbours, linked_neighbour->node);
            linked_neighbour = linked_neighbour->next;
        }

        close_list = insert_into_list(close_list, &current_node);

        int i = 0;
        // check neighbours
        // if already checked, skip it
        // if never seen, add to priority queue
        // else check if distance is shorter than previous one
        while(i < close_list->used) {
            if (find((Node**) close_list->arr, close_list->used, neighbours->arr[i])) {
                continue;
            } else if (!find(open_list->arr, open_list->size, neighbours->arr[i])) {
                insert_into_heap(open_list, *(Node*) neighbours->arr[i]);
            } else {
                Node* inserted_node = get_node(open_list, neighbours->arr[i]);

                double normal_distance = compute_heuristic(*(Node*) neighbours->arr[i], destination_node);

                if (normal_distance < inserted_node->normal_distance){
                    inserted_node->normal_distance = normal_distance;
                    inserted_node->distance = inserted_node->heuristic_distance + normal_distance;
                    // open_list = update_priority(open_list, inserted_node);
                }
            }
            i++;
        }
    }
    return -1;
}

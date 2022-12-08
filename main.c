#include "main.h"

int main() {
    // todo: read input graph -> read_data.c
    // read graph
    Node* graph;

    // select start and end nodes
    Node starting_node;
    Node destination;

    // initialize list for node seen and to see
    MinHeap* open_list = init_minheap(0);
    List* close_list = init_list(0);

    // initialize starting node
    starting_node.normal_distance = 0;
    starting_node.heuristic_distance = compute_heuristic(graph, starting_node, destination);
    starting_node.distance = starting_node.normal_distance;
    open_list = insert_into_heap(open_list, starting_node);


    while (is_empty(open_list)){
        // get first element in queue
        Node current_node = delete_min(open_list);

        if (current_node.value == destination.value){
            print_list("%s", close_list->arr);
            return 0;
        }

        // get neighbouring nodes
        Node* neighbours = get_neighbours(graph, &current_node);

        close_list = insert_into_list(close_list, &current_node);

        int i = 0;
        // check neighbours
        // if already checked, skip it
        // if never seen, add to priority queue
        // else check if distance is shorter than previous one
        while(i < close_list->len){
            if (find(close_list->arr, close_list->len, &neighbours[i])){
                continue;
            } else if (!find(open_list->arr, open_list->size, &neighbours[i])){
                insert_into_heap(open_list, neighbours[i]);
            } else {
                Node* inserted_node = get_node(open_list, neighbours[i]);

                double normal_distance = compute_heuristic(neighbours[i]);

                if (normal_distance < inserted_node->normal_distance){
                    inserted_node->normal_distance = normal_distance;
                    inserted_node->distance = inserted_node->heuristic_distance + normal_distance;
                    inserted_node->parent = neighbours[i].parent;
                    open_list = update_priority(open_list, inserted_node);
                }
            }
            i++;
        }
    }
    return -1;
}

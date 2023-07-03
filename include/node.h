//
// Created by Jacopo Clocchiatti on 07/12/22.
//

#ifndef NODE_H
#define NODE_H
typedef struct Node Node;

typedef enum {start, end, cell, obstacle, visited} NodeType;

struct Node{
    int id;
    double distance;
    double normal_distance;
    double heuristic_distance;
    int x; // needed for a generic domain
    int y; // needed for a generic domain
    NodeType type;
};

#endif
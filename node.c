//
// Created by Jacopo Clocchiatti on 07/12/22.
//


typedef struct Node Node;


struct Node{
    char value;
    double distance;
    double normal_distance;
    double heuristic_distance;
    Node* parent;

};
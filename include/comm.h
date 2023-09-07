///
/// This file defines and shared entities
/// between the root rank and the others
///

#ifndef PARALLEL_A_STAR_NEW_COMM_H
#define PARALLEL_A_STAR_NEW_COMM_H

#define N_EXIT_POINTS_PER_CHUNK 16 // must be multiple of 4
#define NULL_COORD -1

typedef struct Coordinates {
    int x;
    int y;
} Coordinates;

/// Message sent to root rank after computation
///
/// n_nodes indicates the number of nodes of the path, useful to iter the array and for heuristic
/// nodes are inclusive of entry and exit points
/// exit_points array has a fixed sides size of 2
typedef struct ChunkPath {
    int n_nodes;
    Coordinates* nodes;
    Coordinates* exit_points;
} ChunkPath;

/// A Node represent a single cell of the gridThe
typedef enum {start, end, cell, obstacle, visited} NodeType;
typedef struct Node {
    int id;
    double score;
    double distance;
    double heuristic;
    Coordinates coordinates;
    NodeType type;
} Node;

typedef struct PriorityQueueNode {
    Node* node;
    double priority;
} PriorityQueueNode;

typedef struct PriorityQueue {
    int size;
    int capacity;
    PriorityQueueNode* nodes;
} PriorityQueue;

/// Message sent to computing ranks with initial data
///
/// Since the matrix should be contiguous the nodes a saved in 1 dimension.
/// A matrix example with * as pointers and + as integers:
/// **++++
///  *++++
///  *++++
/// This Node **matrix would have chunk_w=4 and chunk_h=3
///
/// starting_point and ending_point could be NULL if not present in the local chunk
/// exit_points array has a fixed size {N_EXIT_POINTS_PER_CHUNK}, some entries could be NULL_COORD
typedef struct MsgChunkStart {
    Node * nodes; // sent with MPI_scatterv
    int chunk_w, chunk_h;
    Coordinates* starting_point;
    Coordinates* ending_point;
    Coordinates exit_points[N_EXIT_POINTS_PER_CHUNK];
    int num_exit_points;
} MsgChunkStart;


/// Message sent to root rank after computation
///
/// paths array has a fixed size {N_EXIT_POINTS_PER_CHUNK}, some entries could be NULL
typedef struct MsgChunkEnd {
    ChunkPath* paths;
    int num_of_paths;
    int num_of_valid_paths;
} MsgChunkEnd;

#endif //PARALLEL_A_STAR_NEW_COMM_H

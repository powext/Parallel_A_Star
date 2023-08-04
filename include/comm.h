///
/// This file defines and shared entities
/// between the root rank and the others
///

#ifndef PARALLEL_A_STAR_NEW_COMM_H
#define PARALLEL_A_STAR_NEW_COMM_H

#define INT N_EXIT_POINTS_PER_CHUNK = 8 // must be multiple of 4

typedef struct Coordinates {
    int x;
    int y;
} Coordinates;

/// Message sent to root rank after computation
///
/// n_nodes indicates the number of nodes of the path, useful to iter the array and for heuristic
/// exit_points array has a fixed sides size of 2
typedef struct ChunkPath {
    int n_nodes;
    Coordinates **nodes;
    Coordinates* exit_points;
} ChunkPath;

/// A Node represent a single cell of the gridThe
typedef enum {start, end, cell, obstacle, visited} NodeType;
typedef struct Node {
    int id;
    double distance;
    double normal_distance;
    double heuristic_distance;
    Coordinates coordinates;
    NodeType type;
} Node;

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
/// exit_points array has a fixed size {N_EXIT_POINTS_PER_CHUNK}, some entries could be NULL
typedef struct MsgChunkStart {
    Node ** matrix;
    int chunk_w, chunk_h;
    Coordinates starting_point, ending_point;
    Coordinates* exit_points;
} MsgChunkStart;


/// Message sent to root rank after computation
///
/// paths array has a fixed size {N_EXIT_POINTS_PER_CHUNK}, some entries could be NULL
struct MsgChunkEnd {
    ChunkPath* paths;
};

#endif //PARALLEL_A_STAR_NEW_COMM_H
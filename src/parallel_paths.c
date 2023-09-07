//
// Created by Jacopo Clocchiatti on 03/09/23.
//
#include "mpi.h"
#include "/usr/local/Cellar/libomp/16.0.6/include/omp.h"
#include <stdlib.h>
#include <printf.h>
#include "../include/comm.h"
#include "../include/parallel_paths.h"
#include "../include/compute_path.h"

extern int DEBUG;

void print_paths(MsgChunkEnd* paths, int my_rank){
    printf("[DEBUG][PROCESS %d] Printing paths!\n", my_rank);

    for (int i = 0; i < paths->num_of_paths; i++){
        printf("[PROCESS %d] [(%d %d) -> (%d %d) = %d] => ", my_rank, paths->paths[i].exit_points[0].x, paths->paths[i].exit_points[0].y, paths->paths[i].exit_points[1].x, paths->paths[i].exit_points[1].y, paths->paths[i].n_nodes);
        for(int j = 0; j < paths->paths[i].n_nodes; j++){
            printf("(%d %d)\t", paths->paths[i].nodes[j].x, paths->paths[i].nodes[j].y);
        }
        printf("[PROCESS %d]\n----------\n", my_rank);
    }
}


/* Compute the path using parallelism if it is needed
 * If both start and end point are in the same chunk the path will be one,
 * If only one is present, compute in parallel the paths from that and each exit points
 * If neither is present, compute the paths between each pair of exit points
 */
MsgChunkEnd* parallel_compute_paths(MsgChunkStart* msg, int rank){
    MsgChunkEnd* paths_found = malloc(sizeof(MsgChunkEnd));
    paths_found->num_of_paths = 0;
    paths_found->num_of_valid_paths = 0;

    // PATHS = (from one exit point to all the others) for each exit points
    //         + from the starting point to each exit points (if starting point is present)
    //         + from each exit points to the destination point (if destination point is present)
    //         + from the starting point to the destination point (if both are in the chunk)
    int total_points_num = msg->num_exit_points;
    int max_paths_num = msg->num_exit_points*(msg->num_exit_points-1); //MAX_EXIT_POINTS*(MAX_EXIT_POINTS-1);
    if (msg->starting_point != NULL){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (msg->ending_point != NULL){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (msg->starting_point != NULL && msg->ending_point != NULL){
        max_paths_num += 1;
    }

    paths_found->paths = malloc(max_paths_num*sizeof(ChunkPath));
    for (int i = 0; i < max_paths_num; i++){
        paths_found->paths[i].nodes = malloc(msg->chunk_w*msg->chunk_h*sizeof (Coordinates));
        paths_found->paths[i].exit_points = malloc(2*sizeof (Coordinates));
        paths_found->paths[i].n_nodes = 0;
    }

    Coordinates* total_points = malloc(total_points_num*sizeof(Coordinates));
    int index = 0;
    int starting_present = 0;
    if (msg->starting_point != NULL){
        total_points[index] = *msg->starting_point;
        index++;
        starting_present++;
    }
    for (index; index < msg->num_exit_points+starting_present; index++){
        total_points[index] = msg->exit_points[index-starting_present];
    }
    if (msg->ending_point != NULL){
        total_points[index] = *msg->ending_point;
    }

    if(DEBUG) {
        for (int i = 0; i < total_points_num; i++) {
            printf("[DEBUG][PROCESS %d] Exit point %d: (%d %d)\n", rank, i, total_points[i].x, total_points[i].y);
        }
    }

    if (DEBUG)
        printf("[DEBUG][PROCESS %d] Finding path between exit points\n", rank);

    int p = 0;
#pragma omp parallel for shared(p, msg, paths_found, total_points_num, total_points) default(none) collapse(2)
    for (int i = 0; i < total_points_num; i++){
        for (int j = i + 1; j < total_points_num; j++){
            if (&total_points[i] != NULL && &total_points[j] != NULL && i != j) {
                ChunkPath *new_path = compute_path(msg->nodes, msg->chunk_w, msg->chunk_h, total_points[i],
                                                   total_points[j]);
#pragma omp critical
                {
                    paths_found->num_of_paths += 1;
                    paths_found->paths[p] = *new_path;
                    if (new_path->n_nodes > 0)
                        paths_found->num_of_valid_paths += 1;
                    p++;
                }
            }
        }
    }

    if (DEBUG)
        print_paths(paths_found, rank);
    free(total_points);
    return paths_found;
}


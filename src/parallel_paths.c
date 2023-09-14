//
// Created by Jacopo Clocchiatti on 03/09/23.
//
#include "omp.h"
#include <stdlib.h>
#include <printf.h>
#include "../include/comm.h"
#include "../include/parallel_paths.h"
#include "../include/compute_path.h"
#include "../include/utility.h"

extern int DEBUG;
extern int DEBUG_PROCESS;

bool is_valid_exit_point_with_constraint(Coordinates exit_point, int side_length){
    if ((exit_point.x >= 0 && exit_point.y >= 0) && (exit_point.x < side_length && exit_point.y < side_length)){
        return true;
    }
    return false;
}


void print_paths(MsgChunkEnd* paths, int my_rank){
    printf_debug("Printing paths\n");

    for (int i = 0; i < paths->num_of_paths; i++){
        printf_debug(" [(%d %d) -> (%d %d) = %d] = > ", paths->paths[i].exit_points[0].x, paths->paths[i].exit_points[0].y, paths->paths[i].exit_points[1].x, paths->paths[i].exit_points[1].y, paths->paths[i].n_nodes);
        for (int j = 0; j < paths->paths[i].n_nodes; j++) {
            if (!DEBUG) continue;
            if (DEBUG_PROCESS > 0 && DEBUG_PROCESS != my_rank) continue;
            printf("(%d %d)\t", paths->paths[i].nodes[j].x, paths->paths[i].nodes[j].y);
        }
        printf_debug("\n");
        printf_debug("----------\n");
    }
}

void write_debug_info(MsgChunkStart* msg, Node* nodes, int size, int rank){
    FILE *logFile;
    time_t currentTime;
    struct tm *localTimeInfo;

    // Get the current time
    time(&currentTime);

    // Convert the current time to local time
    localTimeInfo = localtime(&currentTime);
    // Create the filename with the rank variable
    char filename[20]; // Adjust the size as needed
    sprintf(filename, "log_%d.txt", rank);

    // Open the log file in append mode
    logFile = fopen(filename, "w");

    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Debug on MsgChunkStart\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Num of exit points: %d\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank, msg->num_exit_points);
    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Exit points: ", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    for (int i = 0; i < msg->num_exit_points; ++i) {
        fprintf(logFile, "(%d:%d) ", msg->exit_points[i].x, msg->exit_points[i].y);
    }
    fprintf(logFile, "\n");
    if(is_valid_exit_point_with_constraint(msg->starting_point, size)){
        fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Start: (%d:%d)\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank, msg->starting_point.x, msg->starting_point.y);
    } else {
        fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Start: NULL\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    }
    if(is_valid_exit_point_with_constraint(msg->ending_point, size)){
        fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] End: (%d:%d)\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank, msg->ending_point.x, msg->ending_point.y);
    } else {
        fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] End: NULL\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    }
    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Dimension: %d x %d\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank, msg->chunk_w, msg->chunk_w);

    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Debug on nodes passed\n", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    fprintf(logFile, "[DEBUG][%02d:%02d:%02d][PROCESS %d] Nodes: ", localTimeInfo->tm_hour, localTimeInfo->tm_min, localTimeInfo->tm_sec, rank);
    for (int i = 0; i < (msg->chunk_w)*(msg->chunk_h); ++i) {
        fprintf(logFile, "%d ", nodes[i].id);
    }
    fprintf(logFile, "\n");

    fclose(logFile);
}

/* Compute the path using parallelism if it is needed
 * If both start and end point are in the same chunk the path will be one,
 * If only one is present, compute in parallel the paths from that and each exit points
 * If neither is present, compute the paths between each pair of exit points
 */
MsgChunkEnd* parallel_compute_paths(MsgChunkStart* msg, Node* elements, int size, int rank){
    if (DEBUG){
        write_debug_info(msg, elements, size, rank);
    }

    MsgChunkEnd* paths_found = malloc(sizeof(MsgChunkEnd));
    paths_found->num_of_paths = 0;
    paths_found->num_of_valid_paths = 0;

    // PATHS = (from one exit point to all the others) for each exit points
    //         + from the starting point to each exit points (if starting point is present)
    //         + from each exit points to the destination point (if destination point is present)
    //         + from the starting point to the destination point (if both are in the chunk)
    int total_points_num = msg->num_exit_points;
    int max_paths_num = msg->num_exit_points*(msg->num_exit_points-1); //MAX_EXIT_POINTS*(MAX_EXIT_POINTS-1);
    if (is_valid_exit_point_with_constraint(msg->starting_point, size)){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (is_valid_exit_point_with_constraint(msg->ending_point, size)){
        max_paths_num += msg->num_exit_points;
        total_points_num += 1;
    }
    if (is_valid_exit_point_with_constraint(msg->starting_point, size) && is_valid_exit_point_with_constraint(msg->ending_point, size)){
        max_paths_num += 1;
    }

    paths_found->paths = malloc(max_paths_num*sizeof(ChunkPath));
    for (int i = 0; i < max_paths_num; i++){
        paths_found->paths[i].n_nodes = 0;
        paths_found->paths[i].exit_points = malloc(2*sizeof (Coordinates));
        paths_found->paths[i].nodes = malloc(msg->chunk_w*msg->chunk_h*sizeof (Coordinates));
    }

    Coordinates* total_points = malloc(total_points_num*sizeof(Coordinates));
    int index = 0;
    int starting_present = 0;
    if (is_valid_exit_point_with_constraint(msg->starting_point, size)){
        total_points[index] = msg->starting_point;
        index++;
        starting_present++;
    }
    for (; index < msg->num_exit_points+starting_present; index++){
        total_points[index] = msg->exit_points[index-starting_present];
    }
    if (is_valid_exit_point_with_constraint(msg->ending_point, size)){
        total_points[index] = msg->ending_point;
    }

    printf_debug("[THREAD %d] Finding path between exit points\n", omp_get_thread_num());

    int p = 0;
#pragma omp parallel for shared(p, elements, size, msg, paths_found, total_points_num, total_points, DEBUG, rank) default(none) collapse(2)
    for (int i = 0; i < total_points_num; i++){
        for (int j = i + 1; j < total_points_num; j++){
            if ((&total_points[i] != NULL && &total_points[j] != NULL) &&
            (is_valid_exit_point_with_constraint(total_points[i], size) && is_valid_exit_point_with_constraint(total_points[j], size)) &&
            i != j) {
                printf_debug("[THREAD %d] Finding path %d between (%d:%d) -> (%d:%d)\n", omp_get_thread_num(), p, total_points[i].x, total_points[i].y, total_points[j].x, total_points[j].y);

                ChunkPath *new_path = compute_path(elements, msg->chunk_w, msg->chunk_h, total_points[i],
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

    print_paths(paths_found, rank);
    free(total_points);
    return paths_found;
}


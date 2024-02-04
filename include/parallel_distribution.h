#include <printf.h>
#include <math.h>
#include <tic.h>
#include <unistd.h>
#include "mpi.h"
#include <stdlib.h>
#include "../include/print.h"
#include "../include/comm.h"
#include "omp.h"
#include "adjlist.h"

//
// Created by Simone Bianchin on 24/08/23.
//

#ifndef PARALLEL_A_STAR_PARALLEL_DISTRIBUTION_H
#define PARALLEL_A_STAR_PARALLEL_DISTRIBUTION_H

MsgChunkEnd* distribute_work(
        Node* nodes,
        AdjList** graph,
        MsgChunkStart** start_msgs,
        int size,
        Node* starting_node,
        Node* destination_node,
        int n_chunks,
        int world_rank,
        clock_t *step_start_time);

#endif //PARALLEL_A_STAR_NEW_EXIT_POINTS_H



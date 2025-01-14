//
// Created by Simone Bianchin on 17/07/23.
//

#ifndef PARALLEL_A_STAR_NEW_PARALLEL_H
#define PARALLEL_A_STAR_NEW_PARALLEL_H

#endif //PARALLEL_A_STAR_NEW_PARALLEL_H
#include "comm.h"

void parallel_init(int* n_chunks, int* world_rank);
void parallel_finalize();
MsgChunkEnd* parallel_compute_paths(MsgChunkStart* msg);

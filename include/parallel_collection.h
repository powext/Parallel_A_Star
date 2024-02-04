//
// Created by Simone Bianchin on 06/09/23.
//

#include "comm.h"
#include "adjlist.h"

#ifndef PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H
#define PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H

MsgChunkEnd *get_dummy_endmsg(int world_rank);

MsgChunkEnd *collect_msgs_end(MsgChunkEnd* msg, Node* nodes, int size, int world_rank, int n_chunks, AdjList* graph);

#endif //PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H

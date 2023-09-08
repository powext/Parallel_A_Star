//
// Created by Simone Bianchin on 06/09/23.
//

#include "comm.h"

#ifndef PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H
#define PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H

MsgChunkEnd *get_dummy_endmsg(int world_rank);

MsgChunkEnd *collect_msgs_end(MsgChunkEnd* msg, int world_rank, int n_chunks);

#endif //PARALLEL_A_STAR_NEW_PARALLEL_COLLECTION_H

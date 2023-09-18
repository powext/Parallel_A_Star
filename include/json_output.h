//
// Created by Simone Bianchin on 25/08/23.
//

#ifndef PARALLEL_A_STAR_NEW_JSON_OUTPUT_H
#define PARALLEL_A_STAR_NEW_JSON_OUTPUT_H

#include "comm.h"

void output_json(Node* nodes, int size, Node* starting_node, Node* destination_node, MsgChunkStart* start_messages, MsgChunkEnd* end_messages, ChunkPath* final_path, int world_rank, int n_chunks);

#endif //PARALLEL_A_STAR_NEW_JSON_OUTPUT_H

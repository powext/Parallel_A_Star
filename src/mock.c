//
// Created by Simone Bianchin on 11/08/23.
//

#include <stdlib.h>
#include "../include/mock.h"
#include "../include/comm.h"

/// Mocked data for matrix_10
void get_msg_chunk_end_mocked(int world_rank) {
    ChunkPath* paths = malloc(sizeof(ChunkPath) * 4);

    if (world_rank == 0) {
        // Paths from start
        paths[0].nodes = malloc(sizeof(Coordinates) * 1);
        paths[0].n_nodes = 1;

        return;
    }
}

#include <stdlib.h>
#include <printf.h>
#include <mpi.h>
#include "../include/comm.h"
#include "../include/parallel_collection.h"
#include "../include/util.h"

MsgChunkEnd *get_dummy_endmsg(int world_rank) {
    MsgChunkEnd* msg = malloc(sizeof(MsgChunkEnd));
    switch (world_rank) {
        case 0:
            printf("[DEBUG] R0 - chunk 0\n");
            msg->paths = malloc(sizeof(ChunkPath) * 2);
            msg->num_of_paths = 2;
            msg->paths[0].n_nodes = 7;
            msg->paths[0].nodes = malloc(sizeof(Coordinates) * msg->paths[0].n_nodes);
            msg->paths[0].nodes[0].x = 8;
            msg->paths[0].nodes[0].y = 6;

            msg->paths[0].nodes[1].x = 7;
            msg->paths[0].nodes[1].y = 6;

            msg->paths[0].nodes[2].x = 6;
            msg->paths[0].nodes[2].y = 6;

            msg->paths[0].nodes[3].x = 6;
            msg->paths[0].nodes[3].y = 7;

            msg->paths[0].nodes[4].x = 5;
            msg->paths[0].nodes[4].y = 7;

            msg->paths[0].nodes[5].x = 5;
            msg->paths[0].nodes[5].y = 8;

            msg->paths[0].nodes[6].x = 5;
            msg->paths[0].nodes[6].y = 9;
            msg->paths[0].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[0].exit_points[0].x = 8;
            msg->paths[0].exit_points[0].y = 6;
            msg->paths[0].exit_points[1].x = 5;
            msg->paths[0].exit_points[1].y = 9;

            msg->paths[1].n_nodes = 3;
            msg->paths[1].nodes = malloc(sizeof(Coordinates) * msg->paths[1].n_nodes);
            msg->paths[1].nodes[0].x = 8;
            msg->paths[1].nodes[0].y = 6;

            msg->paths[1].nodes[1].x = 8;
            msg->paths[1].nodes[1].y = 5;

            msg->paths[1].nodes[2].x = 9;
            msg->paths[1].nodes[2].y = 5;
            msg->paths[1].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[1].exit_points[0].x = 8;
            msg->paths[1].exit_points[0].y = 6;
            msg->paths[1].exit_points[1].x = 9;
            msg->paths[1].exit_points[1].y = 5;
            break;
        case 1:
            msg->paths = malloc(sizeof(ChunkPath) * 1);
            msg->num_of_paths = 1;
            msg->paths[0].n_nodes = 16;
            msg->paths[0].nodes = malloc(sizeof(Coordinates) * msg->paths[0].n_nodes);
            msg->paths[0].nodes[0].x = 10;
            msg->paths[0].nodes[0].y = 4;

            msg->paths[0].nodes[1].x = 10;
            msg->paths[0].nodes[1].y = 3;

            msg->paths[0].nodes[2].x = 11;
            msg->paths[0].nodes[2].y = 3;

            msg->paths[0].nodes[3].x = 12;
            msg->paths[0].nodes[3].y = 3;

            msg->paths[0].nodes[4].x = 13;
            msg->paths[0].nodes[4].y = 3;

            msg->paths[0].nodes[5].x = 14;
            msg->paths[0].nodes[5].y = 3;

            msg->paths[0].nodes[6].x = 15;
            msg->paths[0].nodes[6].y = 3;

            msg->paths[0].nodes[7].x = 16;
            msg->paths[0].nodes[7].y = 3;

            msg->paths[0].nodes[8].x = 16;
            msg->paths[0].nodes[8].y = 4;

            msg->paths[0].nodes[9].x = 16;
            msg->paths[0].nodes[9].y = 5;

            msg->paths[0].nodes[10].x = 16;
            msg->paths[0].nodes[10].y = 6;

            msg->paths[0].nodes[11].x = 16;
            msg->paths[0].nodes[11].y = 7;

            msg->paths[0].nodes[12].x = 17;
            msg->paths[0].nodes[12].y = 7;

            msg->paths[0].nodes[13].x = 16;
            msg->paths[0].nodes[13].y = 8;

            msg->paths[0].nodes[14].x = 16;
            msg->paths[0].nodes[14].y = 9;

            msg->paths[0].nodes[15].x = 15;
            msg->paths[0].nodes[15].y = 9;
            msg->paths[0].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[0].exit_points[0].x = 10;
            msg->paths[0].exit_points[0].y = 4;
            msg->paths[0].exit_points[1].x = 15;
            msg->paths[0].exit_points[1].y = 9;
            break;
        case 2:
            msg->paths = malloc(sizeof(ChunkPath) * 2);
            msg->num_of_paths = 2;
            msg->paths[0].n_nodes = 12;
            msg->paths[0].nodes = malloc(sizeof(Coordinates) * msg->paths[0].n_nodes);
            msg->paths[0].nodes[0].x = 5;
            msg->paths[0].nodes[0].y = 9;

            msg->paths[0].nodes[1].x = 5;
            msg->paths[0].nodes[1].y = 10;

            msg->paths[0].nodes[2].x = 4;
            msg->paths[0].nodes[2].y = 10;

            msg->paths[0].nodes[3].x = 4;
            msg->paths[0].nodes[3].y = 11;

            msg->paths[0].nodes[4].x = 4;
            msg->paths[0].nodes[4].y = 12;

            msg->paths[0].nodes[5].x = 4;
            msg->paths[0].nodes[5].y = 13;

            msg->paths[0].nodes[6].x = 4;
            msg->paths[0].nodes[6].y = 14;

            msg->paths[0].nodes[7].x = 4;
            msg->paths[0].nodes[7].y = 15;

            msg->paths[0].nodes[8].x = 5;
            msg->paths[0].nodes[8].y = 15;

            msg->paths[0].nodes[9].x = 6;
            msg->paths[0].nodes[9].y = 15;

            msg->paths[0].nodes[10].x = 7;
            msg->paths[0].nodes[10].y = 15;

            msg->paths[0].nodes[11].x = 8;
            msg->paths[0].nodes[11].y = 15;
            msg->paths[0].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[0].exit_points[0].x = 5;
            msg->paths[0].exit_points[0].y = 9;
            msg->paths[0].exit_points[1].x = 8;
            msg->paths[0].exit_points[1].y = 15;

            msg->paths[1].n_nodes = 3;
            msg->paths[1].nodes = malloc(sizeof(Coordinates) * msg->paths[1].n_nodes);
            msg->paths[1].nodes[0].x = 8;
            msg->paths[1].nodes[0].y = 15;

            msg->paths[1].nodes[1].x = 8;
            msg->paths[1].nodes[1].y = 14;

            msg->paths[1].nodes[2].x = 9;
            msg->paths[1].nodes[2].y = 14;
            msg->paths[1].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[1].exit_points[0].x = 8;
            msg->paths[1].exit_points[0].y = 15;
            msg->paths[1].exit_points[1].x = 9;
            msg->paths[1].exit_points[1].y = 14;
            break;
        case 3:
            msg->paths = malloc(sizeof(ChunkPath) * 1);
            msg->num_of_paths = 1;
            msg->paths[0].n_nodes = 12;
            msg->paths[0].nodes = malloc(sizeof(Coordinates) * msg->paths[0].n_nodes);
            msg->paths[0].nodes[0].x = 15;
            msg->paths[0].nodes[0].y = 10;

            msg->paths[0].nodes[1].x = 15;
            msg->paths[0].nodes[1].y = 11;

            msg->paths[0].nodes[2].x = 15;
            msg->paths[0].nodes[2].y = 12;

            msg->paths[0].nodes[3].x = 15;
            msg->paths[0].nodes[3].y = 13;

            msg->paths[0].nodes[4].x = 15;
            msg->paths[0].nodes[4].y = 14;

            msg->paths[0].nodes[5].x = 15;
            msg->paths[0].nodes[5].y = 15;

            msg->paths[0].nodes[6].x = 15;
            msg->paths[0].nodes[6].y = 16;

            msg->paths[0].nodes[7].x = 14;
            msg->paths[0].nodes[7].y = 16;

            msg->paths[0].nodes[8].x = 13;
            msg->paths[0].nodes[8].y = 16;

            msg->paths[0].nodes[9].x = 12;
            msg->paths[0].nodes[9].y = 16;

            msg->paths[0].nodes[10].x = 11;
            msg->paths[0].nodes[10].y = 16;

            msg->paths[0].nodes[11].x = 10;
            msg->paths[0].nodes[11].y = 16;
            msg->paths[0].exit_points = malloc(sizeof(Coordinates) * 2);
            msg->paths[0].exit_points[0].x = 15;
            msg->paths[0].exit_points[0].y = 10;
            msg->paths[0].exit_points[1].x = 10;
            msg->paths[0].exit_points[1].y = 16;
            break;
    }
    return msg;
}

int packMsgChunkEnd(MsgChunkEnd *msg, void *buf, int bufsize) {
    int position = 0;
    MPI_Pack(&(msg->num_of_paths), 1, MPI_INT, buf, bufsize, &position, MPI_COMM_WORLD);
    for (int i = 0; i < msg->num_of_paths; i++) {
        MPI_Pack(&(msg->paths[i].n_nodes), 1, MPI_INT, buf, bufsize, &position, MPI_COMM_WORLD);
        MPI_Pack(msg->paths[i].nodes, msg->paths[i].n_nodes * 2, MPI_INT, buf, bufsize, &position, MPI_COMM_WORLD);
        MPI_Pack(msg->paths[i].exit_points, 4, MPI_INT, buf, bufsize, &position, MPI_COMM_WORLD);
    }
    return position; // Return the size of packed data
}

MsgChunkEnd *collect_msgs_end(MsgChunkEnd* msg, int world_rank, int n_chunks) {
    int EXIT_POINTS_PER_PATH = 2;

    int bufsize = MPI_BSEND_OVERHEAD;
    for (int i = 0; i < msg->num_of_paths; i++) {
        bufsize += (1 + msg->paths[i].n_nodes * 2 + EXIT_POINTS_PER_PATH * 2) * sizeof(int);
    }
    void *buf = malloc(bufsize);
    int packedSize = packMsgChunkEnd(msg, buf, bufsize);

    int *allSizes = NULL;
    if (world_rank== 0) {
        allSizes = (int *)malloc(n_chunks * sizeof(int));
    }
    MPI_Gather(&packedSize, 1, MPI_INT, allSizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int *displacements = NULL;
    if (world_rank == 0) {
        displacements = (int *)malloc(n_chunks * sizeof(int));
        displacements[0] = 0;
        for (int i = 1; i < n_chunks; i++) {
            displacements[i] = displacements[i - 1] + allSizes[i - 1];
        }
    }
    int totalDataSize = 0;
    if (world_rank == 0)
        for (int i = 0; i < n_chunks; i++)
            totalDataSize += allSizes[i];
    void *allData = NULL;
    if (world_rank == 0) {
        allData = malloc(totalDataSize);
    }
    MPI_Gatherv(buf, packedSize, MPI_PACKED, allData, allSizes, displacements, MPI_PACKED, 0, MPI_COMM_WORLD);

    MsgChunkEnd *receivedMsgs = NULL;
    if (world_rank == 0) {
        receivedMsgs = (MsgChunkEnd *)malloc(n_chunks * sizeof(MsgChunkEnd));
        int position = 0;
        for (int i = 0; i < n_chunks; i++) {
            MPI_Unpack(allData, totalDataSize, &position, &(receivedMsgs[i].num_of_paths), 1, MPI_INT, MPI_COMM_WORLD);
            receivedMsgs[i].paths = (ChunkPath *)malloc(receivedMsgs[i].num_of_paths * sizeof(ChunkPath));
            for (int j = 0; j < receivedMsgs[i].num_of_paths; j++) {
                MPI_Unpack(allData, totalDataSize, &position, &(receivedMsgs[i].paths[j].n_nodes), 1, MPI_INT, MPI_COMM_WORLD);
                receivedMsgs[i].paths[j].nodes = (Coordinates *)malloc(receivedMsgs[i].paths[j].n_nodes * sizeof(Coordinates));
                MPI_Unpack(allData, totalDataSize, &position, receivedMsgs[i].paths[j].nodes, receivedMsgs[i].paths[j].n_nodes * 2, MPI_INT, MPI_COMM_WORLD);
                receivedMsgs[i].paths[j].exit_points = (Coordinates *)malloc(2 * sizeof(Coordinates));
                MPI_Unpack(allData, totalDataSize, &position, receivedMsgs[i].paths[j].exit_points, 4, MPI_INT, MPI_COMM_WORLD);
            }
        }
    }

    // Clean up
    free(buf);
    if (world_rank == 0) {
        free(allSizes);
        free(displacements);
        free(allData);
    }
    return receivedMsgs;
}

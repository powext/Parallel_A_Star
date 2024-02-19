#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include "../include/json_output.h"
#include "../include/cJSON.h"
#include "../include/utility.h"

extern int N_EXIT_POINTS_PER_CHUNK;

void writeOutputToFile(cJSON *json_object) {
    struct stat st = {0};
    if (stat("output", &st) == -1) {
        mkdir("output", 0700);
    }

    FILE *file = fopen("output/output.json", "w");
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }

    char *string = cJSON_Print(json_object);
    if (string == NULL) {
        fprintf(stderr, "Failed to print cJSON.\n");
        fclose(file);
        return;
    }
    fprintf(file, "%s", string);
    free(string);

    fclose(file);
}

char *createCompactCoordinatesOutput(Coordinates *coords, int size) {
    int max_int_length = snprintf(NULL, 0, "%d", INT_MAX);
    int buffer_size = size * (2 * max_int_length + 2);

    char* output = (char*)malloc(buffer_size * sizeof(char));
    char buffer[2 * max_int_length + 3]; // +2 for commas, +1 for null terminator

    int offset = 0;

    for (int i = 0; i < size; i++) {
        sprintf(buffer, "%d,%d,", coords[i].x, coords[i].y);
        strcpy(output + offset, buffer);
        offset += strlen(buffer);
    }

    // Remove the trailing comma
    if (offset > 0) {
        output[offset - 1] = '\0';
    } else {
        output[0] = '\0';
    }

    return output;
}

char *createCompactMatrixOutput(Node *matrix, int size) {
    // Initial size for the dynamic string
    int bufferSize = 100;
    char *output = (char *)malloc(bufferSize * sizeof(char));
    if (!output) {
        perror("Failed to allocate memory for output string");
        exit(1);
    }
    output[0] = '\0';

    for (int i = 0; i < size; i++) {
        if (matrix[i].type == obstacle) {
            // Calculate the space needed for the current id and a comma
            int spaceNeeded = snprintf(NULL, 0, "%d,", matrix[i].id) + 1;

            // Check if the current buffer size is enough
            while (strlen(output) + spaceNeeded >= bufferSize) {
                bufferSize *= 2;
                output = (char *)realloc(output, bufferSize * sizeof(char));
                if (!output) {
                    perror("Failed to reallocate memory for output string");
                    exit(1);
                }
            }

            // Append the id to the output string
            sprintf(output + strlen(output), "%d,", matrix[i].id);
        }
    }

    if (strlen(output) == 0) {
        free(output);
        return "";
    }
    if (output[strlen(output) - 1] == ',') {
        output[strlen(output) - 1] = '\0';
    }

    return output;
}

void output_json(
        Node* nodes,
        int size,
        Node* starting_node,
        Node *destination_node,
        MsgChunkStart* start_messages,
        MsgChunkEnd* end_messages,
        ChunkPath* final_path,
        int world_rank,
        int n_chunks
) {
    if (world_rank != 0) return;

    cJSON *c_root = cJSON_CreateObject();

    cJSON_AddItemToObject(c_root, "n_chunks", cJSON_CreateNumber(n_chunks));
    cJSON_AddItemToObject(c_root, "grid_size", cJSON_CreateNumber(size));
    cJSON_AddItemToObject(c_root, "obstacles",
                          cJSON_CreateString(createCompactMatrixOutput(nodes, size * size)));

    // print destination_node coordinates
    printf_debug("[DEBUG] R%d - destination_node: %d:%d\n", world_rank, destination_node->coordinates.x,
           destination_node->coordinates.y);

    char *starting_point_flattened = malloc(sizeof(char) * (size * 2) + 1);
    sprintf(starting_point_flattened, "%d,%d", starting_node->coordinates.x, starting_node->coordinates.y);
    cJSON_AddItemToObject(c_root, "starting_point", cJSON_CreateString(starting_point_flattened));
    char *ending_point_flattened = malloc(sizeof(char) * (size * 2) + 1);
    sprintf(ending_point_flattened, "%d,%d", destination_node->coordinates.x, destination_node->coordinates.y);
    cJSON_AddItemToObject(c_root, "destination_point", cJSON_CreateString(ending_point_flattened));

    cJSON *c_chunk_info = cJSON_CreateArray();
    for (int i = 0; i < n_chunks; i++) {
        cJSON *c_chunk = cJSON_CreateObject();
        cJSON_AddItemToObject(c_chunk, "exit_points", cJSON_CreateString(
                createCompactCoordinatesOutput(start_messages[i].exit_points, N_EXIT_POINTS_PER_CHUNK)));
        cJSON *c_chunk_paths = cJSON_CreateArray();
        for (int j = 0; j < end_messages[i].num_of_paths; j++) {
            cJSON_AddItemToArray(c_chunk_paths, cJSON_CreateString(
                    createCompactCoordinatesOutput(end_messages[i].paths[j].nodes, end_messages[i].paths[j].n_nodes)));
        }
        cJSON_AddItemToObject(c_chunk, "paths", c_chunk_paths);
        cJSON_AddItemToArray(c_chunk_info, c_chunk);
    }
    cJSON_AddItemToObject(c_root, "chunks", c_chunk_info);

    cJSON_AddItemToObject(c_root, "final_path", cJSON_CreateString(createCompactCoordinatesOutput(final_path->nodes, final_path->n_nodes)));

    writeOutputToFile(c_root);
}

#include <stdarg.h>
#include <printf.h>

#include <unistd.h>
#include <mpi.h>

extern int DEBUG;
extern int HEIGHT;

int printf_debug(const char *format, ...) {

    printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);

    return result;
}

void debug(int rank_to_debug) {
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    if (world_rank != rank_to_debug) return;
    volatile int i = 0;
    while (0 == i)
        sleep(5);
}

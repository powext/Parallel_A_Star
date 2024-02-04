//
// Created by Simone Bianchin on 31/08/23.
//


#ifndef PARALLEL_A_STAR_NEW_UTILITY_H
#define PARALLEL_A_STAR_NEW_UTILITY_H

#include "comm.h"

int printf_debug(const char *format, ...);
void debug(int rank_to_debug);
void printf_with_colors(Node c);

#endif //PARALLEL_A_STAR_NEW_UTILITY_H


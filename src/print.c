#include <stdio.h>
#include "../include/colors.h"
#include "../include/print.h"
#include "../include/comm.h"

void printf_with_colors(Node c) {
    if (c.type == cell)
        printf(GREY "+" RESET, (int)c.distance);
    else if (c.type == start)
        printf(GREEN "S" RESET);
    else if (c.type == end)
        printf(RED "E" RESET);
    else if (c.type == obstacle)
        printf(WHITE "-" RESET, (int)c.distance);
    else if (c.type == visited)
        printf(BLUE "+" RESET, (int)c.distance);
}

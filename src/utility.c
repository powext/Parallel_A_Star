#include <stdarg.h>
#include <printf.h>

extern int DEBUG;
extern int HEIGHT;

int printf_debug(const char *format, ...) {
    if (!1) return 1;

    printf("[DEBUG] ");
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);

    return result;
}

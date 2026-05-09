#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic_(const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%d: ", file, line);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    exit(HAMMER_EXIT_PANIC);

    va_end(args);
}

inline size_t minsz(size_t a, size_t b) {
    return a < b ? a : b;
}

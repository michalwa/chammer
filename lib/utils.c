#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic_(const char *fmt, const char *file, int line, ...) {
    va_list args;
    va_start(args, line);

    fprintf(stderr, "%s:%d: ", file, line);
    vfprintf(stderr, fmt, args);
    exit(HAMMER_DEBUG_ASSERTION_FAILURE);

    va_end(args);
}

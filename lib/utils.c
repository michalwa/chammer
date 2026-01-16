#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic_(const char *file, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "%s:%d: ", file, line);
    vfprintf(stderr, fmt, args);
    exit(HAMMER_EXIT_PANIC);

    va_end(args);
}

#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(stderr, fmt, args);
    exit(HAMMER_DEBUG_ASSERTION_FAILURE);

    va_end(args);
}

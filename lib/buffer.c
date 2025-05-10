#include "buffer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_DEFAULT_CAPACITY 0x400

void buffer_init(Buffer *b) {
    buffer_init_capacity(b, BUFFER_DEFAULT_CAPACITY);
}

void buffer_init_capacity(Buffer *b, size_t capacity) {
    b->capacity = capacity;
    b->len = 0;
    b->data = malloc(b->capacity);
}

void buffer_printf(Buffer *b, const char *format, ...) {
    va_list args;
    va_start(args, format);

    size_t free = b->capacity - b->len;
    size_t written = vsnprintf(b->data + b->len, free, format, args);

    if (written > free) {
        b->capacity <<= 1;
        b->data = realloc(b->data, b->capacity);

        free = b->capacity - b->len;
        written = vsnprintf(b->data + b->len, free, format, args);
    }

    b->len += written;
}

void buffer_free(Buffer *b) {
    free(b->data);
}

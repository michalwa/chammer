#include "buffer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_INITIAL_CAPACITY 0x400

void buffer_init(Buffer *b) {
    b->capacity = BUFFER_INITIAL_CAPACITY;
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

string buffer_alloc(Buffer *b, size_t len) {
    if (b->len + len > b->capacity) {
        b->capacity <<= 1;
        b->data = realloc(b->data, b->capacity);
    }

    string item = {
        .data = b->data + b->len,
        .len = len,
    };
    b->len += len;
    return item;
}

void buffer_free(Buffer *b) {
    free(b->data);
}

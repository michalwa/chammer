#include "buffer.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_DEFAULT_CAPACITY 0x400

static inline bool buffer_grow(Buffer *b, size_t min_len) {
    size_t old_capacity = b->capacity;

    while (min_len > b->capacity) {
        b->capacity <<= 1;
        b->data = realloc(b->data, b->capacity);
    }

    return b->capacity > old_capacity;
}

void buffer_init(Buffer *b) {
    buffer_init_capacity(b, BUFFER_DEFAULT_CAPACITY);
}

void buffer_init_capacity(Buffer *b, size_t capacity) {
    b->capacity = capacity;
    b->len = 0;
    b->data = malloc(b->capacity);
}

void buffer_putc(Buffer *b, char c) {
    buffer_grow(b, b->len + 1);
    b->data[b->len++] = c;
}

void buffer_printf(Buffer *b, const char *format, ...) {
    va_list args;
    va_start(args, format);

    size_t len;
    do len = vsnprintf(b->data + b->len, b->capacity - b->len, format, args);
    while (buffer_grow(b, b->len + len));

    b->len += len;

    va_end(args);
}

char *buffer_alloc(Buffer *b, size_t len) {
    buffer_grow(b, b->len + len);

    char *c = b->data + b->len;
    b->len += len;
    return c;
}

void buffer_free(Buffer *b) {
    free(b->data);
}

inline string buffer_to_string(Buffer b) {
    return (string){ .data = b.data, .len = b.len };
}

void fread_to_buffer(FILE *f, Buffer *b) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char  *data = buffer_alloc(b, size);

    rewind(f);
    fread(data, size, 1, f);
}

#include "buffer.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_DEFAULT_CAPACITY 0x400

/*
 * `min_len` does not include the null terminator
 */
static bool buffer_grow(Buffer *b, size_t min_len) {
    size_t old_capacity = b->capacity;

    while (min_len + 1 > b->capacity) {
        b->capacity <<= 1;
        b->data = realloc(b->data, b->capacity);
    }

    return b->capacity > old_capacity;
}

inline void buffer_init(Buffer *b) {
    buffer_init_capacity(b, BUFFER_DEFAULT_CAPACITY);
}

void buffer_init_capacity(Buffer *b, size_t capacity) {
    b->capacity = capacity;
    b->len = 0;
    b->data = malloc(b->capacity);
    b->data[b->len] = '\0';
}

void buffer_free(Buffer *b) {
    free(b->data);
}

void buffer_putc(Buffer *b, char c) {
    buffer_grow(b, b->len + 1);
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
}

inline void buffer_puts(Buffer *b, const char *str, size_t len) {
    memcpy(buffer_alloc(b, len), str, len);
}

void buffer_printf(Buffer *b, const char *format, ...) {
    va_list args;

    size_t len;
    do {
        va_start(args, format);
        len = vsnprintf(b->data + b->len, b->capacity - b->len, format, args);
        va_end(args);
    } while (buffer_grow(b, b->len + len));

    b->len += len;
    b->data[b->len] = '\0';
}

char *buffer_alloc(Buffer *b, size_t len) {
    buffer_grow(b, b->len + len);

    char *c = b->data + b->len;
    b->len += len;
    b->data[b->len] = '\0';
    return c;
}

void buffer_clear(Buffer *b) {
    b->len = 0;
    b->data[b->len] = '\0';
}

void buffer_read_file(Buffer *b, FILE *f) {
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    char  *data = buffer_alloc(b, size);

    rewind(f);

    // `fread` performs line ending conversion on Windows, so we have to trim
    // the buffer just in case. Generally it's probably best to use `rb` instead
    // of `r` anyway
    size_t actual_size = fread(data, 1, size, f);
    b->len -= size - actual_size;
    b->data[b->len] = '\0';
}

#include "strings.h"
#include <stdlib.h>

#define STRINGS_INITIAL_CAPACITY 1024

void strings_init(Strings *s) {
    s->capacity = STRINGS_INITIAL_CAPACITY;
    s->buffer = malloc(s->capacity);
    s->cursor = 0;
}

str strings_alloc(Strings *s, size_t len) {
    if (s->cursor + len > s->capacity) {
        s->capacity << 1;
        s->buffer = realloc(s->buffer, s->capacity);
    }

    str item = {
        .data = s->buffer + s->cursor,
        .len = len,
    };
    s->cursor += len;
    return item;
}

void strings_free(Strings *s) {
    free(s->buffer);
}

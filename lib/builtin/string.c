#include "string.h"

#include <stdlib.h>

typedef struct {
    size_t rc;
    Buffer buffer;
} ConcatBuffer;

static HValue string_concat_call(const void *data, const HValue *args, Machine *m) {
    (void)m;

    ConcatBuffer *b = (ConcatBuffer *)data;
    buffer_clear(&b->buffer);
    buffer_puts(&b->buffer, hvalue_string_get(&args[0]));
    buffer_puts(&b->buffer, hvalue_string_get(&args[1]));
    return hvalue_make_string(buffer_string(&b->buffer));
}

static void *string_concat_clone(const void *data) {
    ((ConcatBuffer *)data)->rc++;
    return (void *)data;
}

static void string_concat_free(void *data) {
    ConcatBuffer *b = (ConcatBuffer *)data;
    if (--b->rc == 0) buffer_free(&b->buffer);
}

static HValue string_substr_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[1].type != V_INT || args[2].type != V_INT) panic("Usage: substr string offset length");

    return hvalue_make_substr(hvalue_ref(&args[0]), args[1].v_int, args[2].v_int);
}

hnative_meta HNATIVE_META_STRING_CONCAT = {
    .name = "++",
    .argc = 2,
    .call = string_concat_call,
    .clone = string_concat_clone,
    .free = string_concat_free,
};

hnative_meta HNATIVE_META_STRING_SUBSTR = {
    .name = "substr",
    .argc = 3,
    .call = string_substr_call,
};

HValue hnative_make_string_concat(void) {
    ConcatBuffer *b = malloc(sizeof(*b));
    b->rc = 1;
    buffer_init(&b->buffer);

    return hvalue_make_native(&HNATIVE_META_STRING_CONCAT, b);
}

HValue hnative_make_string_substr(void) {
    return hvalue_make_native(&HNATIVE_META_STRING_SUBSTR, NULL);
}

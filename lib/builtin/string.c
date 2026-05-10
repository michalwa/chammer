#include "string.h"

static HValue repr_call(const void *data, const HValue *args, Machine *m) {
    (void)data;

    buffer_clear(&m->shared_buffer);
    hvalue_print_repr(&args[0], &m->shared_buffer, m);
    return hvalue_make_string(buffer_string(&m->shared_buffer));
}

static HValue string_concat_call(const void *data, const HValue *args, Machine *m) {
    (void)data;

    buffer_clear(&m->shared_buffer);
    buffer_puts(&m->shared_buffer, hvalue_string_get(&args[0]));
    buffer_puts(&m->shared_buffer, hvalue_string_get(&args[1]));
    return hvalue_make_string(buffer_string(&m->shared_buffer));
}

static HValue substr_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[1].type != V_INT || args[2].type != V_INT) panic("Usage: substr string offset length");

    return hvalue_make_substr(hvalue_ref(&args[0]), args[1].v_int, args[2].v_int);
}

hnative_meta HNATIVE_META_REPR = {
    .name = "repr",
    .argc = 1,
    .call = repr_call,
};

hnative_meta HNATIVE_META_STRING_CONCAT = {
    .name = "++",
    .argc = 2,
    .call = string_concat_call,
};

hnative_meta HNATIVE_META_SUBSTR = {
    .name = "substr",
    .argc = 3,
    .call = substr_call,
};

HValue hnative_make_repr(void) {
    return hvalue_make_native(&HNATIVE_META_REPR, NULL);
}

HValue hnative_make_string_concat(void) {
    return hvalue_make_native(&HNATIVE_META_STRING_CONCAT, NULL);
}

HValue hnative_make_substr(void) {
    return hvalue_make_native(&HNATIVE_META_SUBSTR, NULL);
}

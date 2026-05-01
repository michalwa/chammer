#include "stdio.h"

#include <stdlib.h>

#include "../utils.h"

static HValue print_call(const void *self, const HValue *args) {
    debug_assert(!self);

    HValue *data = malloc(sizeof(HValue));
    *data = hvalue_ref(&args[0]);
    return hvalue_make_native(&HNATIVE_META_PRINT, data);
}

static void print_free(void *data) {
    if (data) {
        hvalue_drop(*(HValue *)data);
        free(data);
    }
}

static void *print_clone(const void *data) {
    if (!data) return NULL;

    HValue *clone = malloc(sizeof(HValue));
    *clone = hvalue_ref((HValue *)data);
    return clone;
}

static void print_print_repr(
    const void *data, const HValue *args, size_t argc, Buffer *out, const program *prog
) {
    (void)args, (void)argc;
    buffer_puts(out, STRING("(print"));

    if (data) {
        buffer_putc(out, ' ');
        hvalue_print_repr((HValue *)data, out, prog);
    }

    buffer_putc(out, ')');
}

hnative_meta HNATIVE_META_PRINT = {
    .name = "print",
    .argc = 1,
    .call = print_call,
    .free = print_free,
    .clone = print_clone,
    .print_repr = print_print_repr,
};

HValue hnative_make_print(void) {
    return hvalue_make_native(&HNATIVE_META_PRINT, NULL);
}

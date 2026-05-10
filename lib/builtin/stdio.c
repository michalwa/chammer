#include "stdio.h"

#include <stdlib.h>

/*
 * The result of calling `print` with an argument. This is the actual effect
 * which accepts yields
 */
typedef struct {
    HValue arg;
} PrintEffect;

extern hnative_meta HNATIVE_META_PRINT_EFFECT;

/*
 * This is the implementation of a call to the `print` function, not a
 * `PrintEffect` value
 */
static HValue print_call(const void *self, const HValue *args, Machine *m) {
    (void)self, (void)m;

    PrintEffect *effect = malloc(sizeof(*effect));
    effect->arg = hvalue_ref(&args[0]);
    return hvalue_make_native(&HNATIVE_META_PRINT_EFFECT, effect);
}

static HValue stdio_print_yield(const void *self, const HValue *then, Machine *m) {
    PrintEffect *effect = (PrintEffect *)self;

    buffer_clear(&m->shared_buffer);
    hvalue_print_repr(&effect->arg, &m->shared_buffer, m);
    printf(F_BUFFER "\n", FA_BUFFER(m->shared_buffer));

    return then ? machine_call(m, hvalue_ref(then), hvalue_make_unit()) : hvalue_make_unit();
}

static void stdio_print_free(void *data) {
    PrintEffect *effect = (PrintEffect *)data;
    hvalue_drop(effect->arg);
    free(effect);
}

static void *stdio_print_clone(const void *data) {
    PrintEffect *clone = malloc(sizeof(*clone));
    *clone = *(PrintEffect *)data;
    return clone;
}

static void stdio_print_print_repr(
    const void *data, const HValue *args, size_t argc, Buffer *out, const Machine *m
) {
    (void)args, (void)argc;
    PrintEffect *effect = (PrintEffect *)data;

    buffer_puts(out, STRING("(print"));

    if (data) {
        buffer_putc(out, ' ');
        hvalue_print_repr(&effect->arg, out, m);
    }

    buffer_putc(out, ')');
}

hnative_meta HNATIVE_META_PRINT = {
    .name = "print",
    .argc = 1,
    .call = print_call,
};

hnative_meta HNATIVE_META_PRINT_EFFECT = {
    .flags = HNATIVE_GENERIC_EFFECT,
    .name = "print",
    .argc = 0,
    .yield = stdio_print_yield,
    .free = stdio_print_free,
    .clone = stdio_print_clone,
    .print_repr = stdio_print_print_repr,
};

HValue hnative_make_print(void) {
    return hvalue_make_native(&HNATIVE_META_PRINT, NULL);
}

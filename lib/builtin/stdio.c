#include "stdio.h"

#include <stdlib.h>

#include "../utils.h"

/*
 * The result of calling `print` with an argument. This is the actual effect
 * which accepts yields
 */
typedef struct {
    HValue arg;
} StdioPrint;

typedef enum {
    STDIO_PRINT, /* StdioPrint */
} stdio_type;

/*
 * A generic wrapper for a standard IO effect bound to a callback function
 */
typedef struct {
    stdio_type type;
    union {
        StdioPrint *print;
    };
    HValue then;
} StdioBinding;

extern hnative_meta HNATIVE_META_STDIO_BINDING;
extern hnative_meta HNATIVE_META_STDIO_PRINT;

/*
 * This is the implementation of a call to the `print` function, not a
 * `StdioPrint` value
 */
static HValue print_call(const void *self, const HValue *args, Machine *m) {
    (void)self, (void)m;

    StdioPrint *effect = malloc(sizeof(*effect));
    effect->arg = hvalue_ref(&args[0]);
    return hvalue_make_native(&HNATIVE_META_STDIO_PRINT, effect);
}

static HValue stdio_print_bind(const void *self, const HValue *then, Machine *m) {
    (void)m;
    StdioPrint *effect = (StdioPrint *)self;

    StdioBinding *binding = malloc(sizeof(*binding));
    binding->type = STDIO_PRINT;
    binding->print = effect;
    binding->then = hvalue_ref(then);
    return hvalue_make_native(&HNATIVE_META_STDIO_BINDING, binding);
}

static HValue stdio_print_yield(const void *self, Machine *m) {
    StdioPrint *effect = (StdioPrint *)self;

    // TODO: Don't allocate this on each invocation
    Buffer output;
    buffer_init(&output);
    hvalue_print_repr(&effect->arg, &output, m);
    printf(F_BUFFER "\n", FA_BUFFER(output));
    buffer_free(&output);

    return hvalue_make_unit();
}

static void stdio_print_free(void *data) {
    StdioPrint *effect = (StdioPrint *)data;
    hvalue_drop(effect->arg);
    free(effect);
}

static void *stdio_print_clone(const void *data) {
    StdioPrint *clone = malloc(sizeof(*clone));
    *clone = *(StdioPrint *)data;
    return clone;
}

static void stdio_print_print_repr(
    const void *data, const HValue *args, size_t argc, Buffer *out, const Machine *m
) {
    (void)args, (void)argc;
    StdioPrint *effect = (StdioPrint *)data;

    buffer_puts(out, STRING("(print"));

    if (data) {
        buffer_putc(out, ' ');
        hvalue_print_repr(&effect->arg, out, m);
    }

    buffer_putc(out, ')');
}

static HValue stdio_binding_yield(const void *data, Machine *m) {
    StdioBinding *binding = (StdioBinding *)data;
    HValue        result;

    switch (binding->type) {
    case STDIO_PRINT: result = stdio_print_yield(binding->print, m); break;
    }

    return machine_call(m, hvalue_ref(&binding->then), result);
}

static void stdio_binding_free(void *data) {
    StdioBinding *binding = (StdioBinding *)data;

    switch (binding->type) {
    case STDIO_PRINT: stdio_print_free(binding->print); break;
    }

    free(binding);
}

static void *stdio_binding_clone(const void *data) {
    StdioBinding *binding = (StdioBinding *)data;
    StdioBinding *clone = malloc(sizeof(*clone));

    clone->then = hvalue_ref(&binding->then);
    clone->type = binding->type;

    switch (binding->type) {
    case STDIO_PRINT: clone->print = stdio_print_clone(&binding->print); break;
    }

    return clone;
}

hnative_meta HNATIVE_META_PRINT = {
    .name = "print",
    .argc = 1,
    .call = print_call,
};

hnative_meta HNATIVE_META_STDIO_PRINT = {
    .name = "<StdioPrint>",
    .argc = 0,
    .bind = stdio_print_bind,
    .yield = stdio_print_yield,
    .free = stdio_print_free,
    .clone = stdio_print_clone,
    .print_repr = stdio_print_print_repr,
};

hnative_meta HNATIVE_META_STDIO_BINDING = {
    .name = "<StdioBinding>",
    .argc = 0,
    .yield = stdio_binding_yield,
    .free = stdio_binding_free,
    .clone = stdio_binding_clone,
};

HValue hnative_make_print(void) {
    return hvalue_make_native(&HNATIVE_META_PRINT, NULL);
}

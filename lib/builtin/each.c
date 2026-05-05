#include "each.h"

#include <stdlib.h>

/*
 * The result of calling `each` with an argument. This is the actual effect
 * which accepts yields
 */
typedef struct {
    HValue list;
} EachEffect;

extern hnative_meta HNATIVE_META_EACH_EFFECT;

static HValue each_call(const void *self, const HValue *args, Machine *m) {
    (void)self, (void)m;

    debug_assert(hvalue_is_list(&args[0]));

    EachEffect *effect = malloc(sizeof(*effect));
    effect->list = hvalue_ref(&args[0]);
    return hvalue_make_native(&HNATIVE_META_EACH_EFFECT, effect);
}

static HValue each_effect_yield(const void *self, const HValue *then, Machine *m) {
    if (!then) return hvalue_make_unit();

    EachEffect *effect = (EachEffect *)self;
    HValue      head, tail = hvalue_ref(&effect->list);

    while (tail.type == V_CONS) {
        hvalue_uncons(tail, &head, &tail);
        HValue effect = machine_call(m, hvalue_ref(then), head);

        HValue result;
        debug_assert(hvalue_yield(&effect, m, &result));
        hvalue_drop(effect);
        hvalue_drop(result);
    }

    hvalue_drop(tail);
    return hvalue_make_unit();
}

static void each_effect_free(void *self) {
    EachEffect *effect = (EachEffect *)self;
    hvalue_drop(effect->list);
    free(effect);
}

static void *each_effect_clone(const void *self) {
    EachEffect *effect = (EachEffect *)self;
    EachEffect *clone = malloc(sizeof(*clone));
    clone->list = hvalue_ref(&effect->list);
    return clone;
}

static void each_effect_print_repr(
    const void *data, const HValue *args, size_t argc, Buffer *out, const Machine *m
) {
    (void)args, (void)argc;
    EachEffect *effect = (EachEffect *)data;

    buffer_puts(out, STRING("(each"));

    if (data) {
        buffer_putc(out, ' ');
        hvalue_print_repr(&effect->list, out, m);
    }

    buffer_putc(out, ')');
}

hnative_meta HNATIVE_META_EACH = {
    .name = "each",
    .argc = 1,
    .call = each_call,
};

hnative_meta HNATIVE_META_EACH_EFFECT = {
    .flags = HNATIVE_GENERIC_EFFECT,
    .name = "each",
    .argc = 0,
    .yield = each_effect_yield,
    .free = each_effect_free,
    .clone = each_effect_clone,
    .print_repr = each_effect_print_repr,
};

HValue hnative_make_each(void) {
    return hvalue_make_native(&HNATIVE_META_EACH, NULL);
}

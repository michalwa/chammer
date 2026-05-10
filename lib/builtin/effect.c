#include "effect.h"

#include <stdlib.h>

extern hnative_meta HNATIVE_META_RETURN_EFFECT;

static HValue return_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    HValue *value = malloc(sizeof(*value));
    *value = hvalue_ref(&args[0]);
    return hvalue_make_native(&HNATIVE_META_RETURN_EFFECT, value);
}

static HValue return_effect_yield(const void *data, const HValue *then, Machine *m) {
    return machine_call(m, hvalue_ref(then), hvalue_ref((HValue *)data));
}

static void return_effect_free(void *data) {
    free(data);
}

static void *return_effect_clone(const void *data) {
    HValue *clone = malloc(sizeof(*clone));
    *clone = *(HValue *)data;
    return clone;
}

hnative_meta HNATIVE_META_RETURN = {
    .name = "return",
    .argc = 1,
    .call = return_call,
};

hnative_meta HNATIVE_META_RETURN_EFFECT = {
    .name = "return",
    .flags = HNATIVE_GENERIC_EFFECT,
    .yield = return_effect_yield,
    .free = return_effect_free,
    .clone = return_effect_clone,
};

HValue hnative_make_return(void) {
    return hvalue_make_native(&HNATIVE_META_RETURN, NULL);
}

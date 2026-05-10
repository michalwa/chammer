#include "value.h"

static HValue eq_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    return hvalue_make_bool(hvalue_eq(&args[0], &args[1]));
}

hnative_meta HNATIVE_META_EQ = {
    .name = "==",
    .argc = 2,
    .call = eq_call,
};

HValue hnative_make_eq(void) {
    return hvalue_make_native(&HNATIVE_META_EQ, NULL);
}

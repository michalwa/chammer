#include "function.h"

static HValue id_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;
    return hvalue_ref(&args[0]);
}

static HValue const_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;
    return hvalue_ref(&args[0]);
}

hnative_meta HNATIVE_META_ID = {
    .name = "id",
    .argc = 1,
    .call = id_call,
};

hnative_meta HNATIVE_META_CONST = {
    .name = "const",
    .argc = 2,
    .call = const_call,
};

HValue hnative_make_id(void) {
    return hvalue_make_native(&HNATIVE_META_ID, NULL);
}

HValue hnative_make_const(void) {
    return hvalue_make_native(&HNATIVE_META_CONST, NULL);
}

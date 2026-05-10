#include "error.h"

typedef struct {
    HValue lhs, rhs;
} AssertEqEffect;

static HValue error_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[0].type != V_STRING) panic("Usage: error message");
    return hvalue_make_error(hvalue_string_get(&args[0]));
}

hnative_meta HNATIVE_META_ERROR = {
    .name = "error",
    .argc = 1,
    .call = error_call,
};

HValue hnative_make_error(void) {
    return hvalue_make_native(&HNATIVE_META_ERROR, NULL);
}

#include "math.h"

static HValue add_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[0].type != V_INT || args[1].type != V_INT)
        panic("(+) currently only supports integer arguments");

    return hvalue_make_int(args[0].v_int + args[1].v_int);
}

static HValue sub_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[0].type != V_INT || args[1].type != V_INT)
        panic("(-) currently only supports integer arguments");

    return hvalue_make_int(args[0].v_int - args[1].v_int);
}

static HValue mul_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[0].type != V_INT || args[1].type != V_INT)
        panic("(*) currently only supports integer arguments");

    return hvalue_make_int(args[0].v_int * args[1].v_int);
}

static HValue div_call(const void *data, const HValue *args, Machine *m) {
    (void)data, (void)m;

    if (args[0].type != V_INT || args[1].type != V_INT)
        panic("(/) currently only supports integer arguments");

    return hvalue_make_int(args[0].v_int / args[1].v_int);
}

hnative_meta HNATIVE_META_ADD = {
    .name = "+",
    .argc = 2,
    .call = add_call,
};

hnative_meta HNATIVE_META_SUB = {
    .name = "-",
    .argc = 2,
    .call = sub_call,
};

hnative_meta HNATIVE_META_MUL = {
    .name = "*",
    .argc = 2,
    .call = mul_call,
};

hnative_meta HNATIVE_META_DIV = {
    .name = "/",
    .argc = 2,
    .call = div_call,
};

HValue hnative_make_add(void) {
    return hvalue_make_native(&HNATIVE_META_ADD, NULL);
}

HValue hnative_make_sub(void) {
    return hvalue_make_native(&HNATIVE_META_SUB, NULL);
}

HValue hnative_make_mul(void) {
    return hvalue_make_native(&HNATIVE_META_MUL, NULL);
}

HValue hnative_make_div(void) {
    return hvalue_make_native(&HNATIVE_META_DIV, NULL);
}

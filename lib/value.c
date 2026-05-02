#include "value.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "machine.h"
#include "utils.h"

struct hvalue_header {
    uint32_t rc;
};

const char *hvalue_type_name(hvalue_type type) {
    RETURN_ENUM_NAME_V(hvalue_type, type, EACH_HVALUE_TYPE);
}

bool hvalue_is_rc(const HValue *hv) {
    switch (hv->type) {
#define TYPE_CASE(name, data_type, data, is_rc, ...) \
    case name: return is_rc;

        EACH_HVALUE_TYPE(TYPE_CASE)
#undef TYPE_CASE
    }
}

inline hvalue_header *hvalue_header_(const HValue *hv) {
    return (hvalue_header *)hv->v_any;
}

inline HValue hvalue_ref(const HValue *hv) {
    if (hvalue_is_rc(hv)) hvalue_header_(hv)->rc++;
    return *hv;
}

static void hclosure_free(HClosure *);
static void hcons_free(HCons *);
static void htuple_free(HTuple *);
static void hnative_free(HNative *);

static void hvalue_free(HValue hv) {
    switch (hv.type) {
#define TYPE_CASE(name, data_type, data, is_rc, free, ...) \
    case name: free(hv.data); break;
        EACH_HVALUE_TYPE(TYPE_CASE)
#undef TYPE_CASE
    }

    free(hv.v_any);
}

void hvalue_drop(HValue hv) {
    if (hvalue_is_rc(&hv)) {
        if (--hvalue_header_(&hv)->rc == 0) hvalue_free(hv);
    }
}

static HValue hstring_clone(const HString *);
static HValue hclosure_clone(const HClosure *);
static HValue hcons_clone(const HCons *);
static HValue htuple_clone(const HTuple *);
static HValue hnative_clone(const HNative *);

/*
 * `clone` function name placeholder for bit-copy types
 */
#define HVALUE_COPY panic("copy of reference-counted value type"), *(HValue *)(intptr_t)

HValue hvalue_clone(const HValue *hv) {
    switch (hv->type) {
#define TYPE_CASE(name, data_type, data, is_rc, free, clone) \
    case name: return is_rc ? clone(hv->data) : *hv;
        EACH_HVALUE_TYPE(TYPE_CASE)
#undef TYPE_CASE
    }
}

inline HValue hvalue_uniq(HValue hv) {
    if (hvalue_is_uniq(&hv)) return hv;

    HValue clone = hvalue_clone(&hv);
    hvalue_drop(hv);
    return clone;
}

inline bool hvalue_is_uniq(const HValue *hv) {
    return !hvalue_is_rc(hv) || hvalue_header_(hv)->rc <= 1;
}

static void hclosure_print_repr(const HClosure *, Buffer *, const machine_ctx *);
static void hcons_print_repr(const HCons *, Buffer *, const machine_ctx *);
static void htuple_print_repr(const HTuple *, Buffer *, const machine_ctx *);
static void hnative_print_repr(const HNative *, Buffer *, const machine_ctx *);

void hvalue_print_repr(const HValue *hv, Buffer *b, const machine_ctx *ctx) {
    switch (hv->type) {
    case V_INT: buffer_printf(b, "%" PRIu32, hv->v_int); break;
    case V_FLOAT: buffer_printf(b, "%lf", hv->v_float); break;
    case V_STRING: buffer_print_string_literal(b, hvalue_string_get(hv)); break;
    case V_CLOSURE: hclosure_print_repr(hv->v_closure, b, ctx); break;
    case V_TRUE: buffer_puts(b, STRING("true")); break;
    case V_FALSE: buffer_puts(b, STRING("false")); break;
    case V_NIL: buffer_puts(b, STRING("[]")); break;
    case V_CONS: hcons_print_repr(hv->v_cons, b, ctx); break;
    case V_TUPLE: htuple_print_repr(hv->v_tuple, b, ctx); break;
    case V_NATIVE: hnative_print_repr(hv->v_native, b, ctx); break;
    }
}

inline HValue hvalue_make(hvalue_type type) {
    switch (type) {
    case V_FALSE:
    case V_TRUE:
    case V_NIL: return (HValue){ .type = type };
    default: panic("`hvalue_make` used with non-primitive type %s", hvalue_type_name(type));
    }
}

inline HValue hvalue_make_bool(bool b) {
    return hvalue_make(b ? V_TRUE : V_FALSE);
}

inline HValue hvalue_make_int(int64_t value) {
    return (HValue){ .type = V_INT, .v_int = value };
}

#define HVALUE_GET(h, t, d, v)  \
    do {                        \
        if ((h)->type == (t)) { \
            *(v) = (h)->d;      \
            return true;        \
        } else {                \
            return false;       \
        }                       \
    } while (0)

inline bool hvalue_get_string(const HValue *hv, const HString **value) {
    HVALUE_GET(hv, V_STRING, v_string, value);
}

inline bool hvalue_get_closure(const HValue *hv, const HClosure **value) {
    HVALUE_GET(hv, V_CLOSURE, v_closure, value);
}

inline bool hvalue_get_cons(const HValue *hv, const HCons **value) {
    HVALUE_GET(hv, V_CONS, v_cons, value);
}

inline bool hvalue_get_tuple(const HValue *hv, const HTuple **value) {
    HVALUE_GET(hv, V_TUPLE, v_tuple, value);
}

inline bool hvalue_get_native(const HValue *hv, const HNative **value) {
    HVALUE_GET(hv, V_NATIVE, v_native, value);
}

#undef HVALUE_GET

#define hvalue_expect(getter, h, v)                                    \
    assert_msg(getter(h, v), "got `%s`", hvalue_type_name((h)->type));

struct HString {
    hvalue_header header;
    uint32_t      len;
    const char    data[];
};

HValue hvalue_make_string(string value) {
    HString *data = malloc(sizeof(HString) + value.len);
    data->header.rc = 1;
    data->len = value.len;
    memcpy((void *)data->data, value.data, value.len);

    return (HValue){ .type = V_STRING, .v_string = data };
}

static HValue hstring_clone(const HString *str) {
    return hvalue_make_string((string){ .data = str->data, .len = str->len });
}

string hvalue_string_get(const HValue *hv) {
    const HString *str;
    hvalue_expect(hvalue_get_string, hv, &str);

    return (string){ .data = str->data, .len = str->len };
}

struct HClosure {
    hvalue_header header;
    uint32_t      fnindex;
    /*
     * Total number of required arguments (captures + args). This could
     * in theory be looked up in the metadata by `fnindex`, but it's easier to
     * have it directly stored as part of the object. This way e.g. cloning
     * doesn't require `machine_ctx`
     */
    uint8_t       argc;
    /*
     * Arguments applied so far, stored in `args`
     */
    uint8_t       args_len;
    HValue        args[];
};

HValue hvalue_make_closure(uint32_t fnindex, uint8_t argc) {
    HClosure *data = malloc(sizeof(HClosure) + sizeof(HValue) * argc);
    data->header.rc = 1;
    data->fnindex = fnindex;
    data->argc = argc;
    data->args_len = 0;

    return (HValue){ .type = V_CLOSURE, .v_closure = data };
}

static void hclosure_free(HClosure *closure) {
    for (uint8_t i = 0; i < closure->args_len; i++) hvalue_drop(closure->args[i]);
}

static HValue hclosure_clone(const HClosure *closure) {
    HValue clone = hvalue_make_closure(closure->fnindex, closure->argc);

    clone.v_closure->args_len = closure->args_len;
    for (uint8_t i = 0; i < closure->args_len; i++)
        clone.v_closure->args[i] = hvalue_ref(&closure->args[i]);

    return clone;
}

static void hclosure_print_repr(const HClosure *closure, Buffer *b, const machine_ctx *ctx) {
    string name = machine_ctx_func_name(ctx, closure->fnindex);

    buffer_printf(b, "(" F_STRING, FA_STRING(name));
    for (uint8_t i = 0; i < closure->args_len; i++) {
        buffer_putc(b, ' ');
        hvalue_print_repr(&closure->args[i], b, ctx);
    }
    buffer_putc(b, ')');
}

uint8_t hvalue_closure_args_left(const HValue *hv) {
    const HClosure *closure;
    hvalue_expect(hvalue_get_closure, hv, &closure);
    return closure->argc - closure->args_len;
}

uint32_t hvalue_closure_fnindex(const HValue *hv) {
    const HClosure *closure;
    hvalue_expect(hvalue_get_closure, hv, &closure);
    return closure->fnindex;
}

void hvalue_closure_put_arg_mut(const HValue *hv, HValue arg) {
    const HClosure *closure;
    hvalue_expect(hvalue_get_closure, hv, &closure);
    debug_assert(hvalue_is_uniq(hv));

    // SAFETY: Ensured unique, safe to mutate
    HClosure *closure_mut = (HClosure *)closure;
    closure_mut->args[closure_mut->args_len++] = arg;
}

bool hvalue_closure_take_arg_mut(const HValue *hv, HValue *arg) {
    const HClosure *closure;
    hvalue_expect(hvalue_get_closure, hv, &closure);
    debug_assert(hvalue_is_uniq(hv));

    if (closure->args_len == 0) return false;

    // SAFETY: Ensured unique, safe to mutate
    HClosure *closure_mut = (HClosure *)closure;
    *arg = closure_mut->args[--closure_mut->args_len];
    return true;
}

struct HCons {
    hvalue_header header;
    HValue        head;
    HValue        tail;
};

HValue hvalue_make_cons(HValue head, HValue tail) {
    HCons *data = malloc(sizeof(HCons));
    data->header.rc = 1;
    data->head = head;
    data->tail = tail;

    return (HValue){ .type = V_CONS, .v_cons = data };
}

static void hcons_free(HCons *cons) {
    hvalue_drop(cons->head);
    hvalue_drop(cons->tail);
}

static HValue hcons_clone(const HCons *cons) {
    return hvalue_make_cons(hvalue_ref(&cons->head), hvalue_ref(&cons->tail));
}

static void hcons_print_repr(const HCons *cons, Buffer *b, const machine_ctx *ctx) {
    buffer_putc(b, '[');

    while (1) {
        hvalue_print_repr(&cons->head, b, ctx);

        switch (cons->tail.type) {
        case V_CONS:
            cons = cons->tail.v_cons;
            buffer_puts(b, STRING(", "));
            break;
        case V_NIL: buffer_putc(b, ']'); return;
        default:
            hvalue_print_repr(&cons->tail, b, ctx);
            buffer_puts(b, STRING(" (malformed list)]"));
            return;
        }
    }
}

void hvalue_uncons(HValue hv, HValue *head, HValue *tail) {
    const HCons *cons;
    hvalue_expect(hvalue_get_cons, &hv, &cons);

    *head = hvalue_ref(&cons->head);
    *tail = hvalue_ref(&cons->tail);
    hvalue_drop(hv);
}

HValue hvalue_list_concat(HValue a, HValue b) {
    if (a.type == V_NIL) return b;
    if (b.type == V_NIL) return a;

    const HCons *cons;
    hvalue_expect(hvalue_get_cons, &a, &cons);

    if (cons->tail.type == V_CONS) b = hvalue_list_concat(hvalue_ref(&cons->tail), b);

    HValue result = hvalue_make_cons(hvalue_ref(&cons->head), b);
    hvalue_drop(a);
    return result;
}

struct HTuple {
    hvalue_header header;
    uint16_t      len;
    HValue        data[];
};

static void htuple_free(HTuple *tuple) {
    for (uint8_t i = 0; i < tuple->len; i++) hvalue_drop(tuple->data[i]);
}

static HValue htuple_clone(const HTuple *tuple) {
    HTupleBuilder clone = htuple_begin(tuple->len);
    for (uint16_t i = 0; i < tuple->len; i++) htuple_put(&clone, hvalue_ref(&tuple->data[i]));
    return htuple_end(clone);
}

static void htuple_print_repr(const HTuple *tuple, Buffer *b, const machine_ctx *ctx) {
    buffer_putc(b, '(');

    for (uint16_t i = 0; i < tuple->len; i++) {
        if (i > 0) buffer_puts(b, STRING(", "));
        hvalue_print_repr(&tuple->data[i], b, ctx);
    }

    if (tuple->len == 1) buffer_putc(b, ',');
    buffer_putc(b, ')');
}

HTupleBuilder htuple_begin(uint16_t len) {
    HTuple *tuple = malloc(sizeof(HTuple) + sizeof(HValue) * len);
    tuple->header.rc = 1;
    tuple->len = len;
    return (HTupleBuilder){ .tuple = tuple, .len = 0 };
}

void htuple_put(HTupleBuilder *tb, HValue item) {
    debug_assert(tb->len < tb->tuple->len);
    tb->tuple->data[tb->len++] = item;
}

HValue htuple_end(HTupleBuilder tb) {
    debug_assert(tb.len == tb.tuple->len);
    return (HValue){ .type = V_TUPLE, .v_tuple = tb.tuple };
}

uint16_t hvalue_tuple_len(const HValue *hv) {
    const HTuple *tuple;
    hvalue_expect(hvalue_get_tuple, hv, &tuple);
    return tuple->len;
}

HValue hvalue_tuple_get(const HValue *hv, uint16_t index) {
    const HTuple *tuple;
    hvalue_expect(hvalue_get_tuple, hv, &tuple);
    return hvalue_ref(&tuple->data[index]);
}

struct HNative {
    hvalue_header       header;
    const hnative_meta *meta;
    void               *data;
    size_t              args_len;
    HValue              args[];
};

HValue hvalue_make_native(const hnative_meta *meta, void *data) {
    HNative *native = malloc(sizeof(HNative) + sizeof(HValue) * meta->argc);
    native->header.rc = 1;
    native->meta = meta;
    native->data = data;
    native->args_len = 0;

    return (HValue){ .type = V_NATIVE, .v_native = native };
}

static void hnative_free(HNative *native) {
    if (native->meta->free) native->meta->free(native->data);
}

static HValue hnative_clone(const HNative *native) {
    void  *data_clone = native->meta->clone ? native->meta->clone(native->data) : native->data;
    HValue clone = hvalue_make_native(native->meta, data_clone);

    for (size_t i = 0; i < native->args_len; i++)
        clone.v_native->args[i] = hvalue_ref(&native->args[i]);

    return clone;
}

static void hnative_print_repr(const HNative *native, Buffer *b, const machine_ctx *ctx) {
    if (native->meta->print_repr) {
        native->meta->print_repr(native->data, native->args, native->args_len, b, ctx);
        return;
    }

    buffer_printf(b, "(%s", native->meta->name);
    for (uint8_t i = 0; i < native->args_len; i++) {
        buffer_putc(b, ' ');
        hvalue_print_repr(&native->args[i], b, ctx);
    }
    buffer_putc(b, ')');
}

size_t hvalue_native_args_left(const HValue *hv) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, hv, &native);
    return native->meta->argc - native->args_len;
}

void hvalue_native_put_arg_mut(const HValue *hv, HValue arg) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, hv, &native);
    debug_assert(hvalue_is_uniq(hv));

    debug_assert(native->args_len < native->meta->argc);

    // SAFETY: Ensured unique, safe to mutate
    HNative *native_mut = (HNative *)native;
    native_mut->args[native_mut->args_len++] = arg;
}

HValue hvalue_native_call(const HValue *hv) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, hv, &native);
    debug_assert(native->args_len == native->meta->argc);
    debug_assert(native->meta->call);

    return native->meta->call(native->data, native->args);
}

#include "value.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "buffer.h"
#include "machine.h"
#include "utils.h"

struct hvalue_header {
    uint32_t rc;
};

const char *hvalue_type_name(hvalue_type type) {
    RETURN_ENUM_NAME_V(hvalue_type, type, EACH_HVALUE_TYPE);
}

inline hvalue_header *hvalue_header_(const HValue *hv) {
    return (hvalue_header *)hv->v_any;
}

inline HValue hvalue_ref(const HValue *hv) {
    switch (hv->type) {
    case V_EMPTY: panic("value is empty");
#define TYPE_CASE(name, data_type, data, strategy) \
    case name: return strategy;
#define _COPY *hv
#define _RC(...)                                                     \
    /* for some RC types like `V_TUPLE` the pointer may be `NULL` */ \
    (hv->v_any && hvalue_header_(hv)->rc++, *hv)

        EACH_HVALUE_TYPE(TYPE_CASE)

#undef TYPE_CASE
#undef _COPY
#undef _RC
    }
}

static void hsubstr_free(HSubstr);
static void hclosure_free(HClosure *);
static void hcons_free(HCons *);
static void htuple_free(HTuple *);
static void hnative_free(HNative *);
static void hbinding_free(HBinding *);

static void hvalue_free(HValue hv) {
    switch (hv.type) {
    case V_EMPTY: panic("value is empty");
#define TYPE_CASE(name, data_type, data, strategy) \
    case name: FORWARD(strategy, data); break;
#define _COPY(data)         /* no-op */
#define _RC(free_fn, ...)   _RC_, free_fn
#define _RC_(free_fn, data) free_fn(hv.data);

        EACH_HVALUE_TYPE(TYPE_CASE)

#undef TYPE_CASE
#undef _COPY
#undef _RC
#undef _RC_
    }
}

void hvalue_drop(HValue hv) {
    switch (hv.type) {
    case V_EMPTY: panic("value is empty");
#define TYPE_CASE(name, data_type, data, strategy) \
    case name: strategy; break;
#define _COPY /* no-op */
#define _RC(...)                                                     \
    /* for some RC types like `V_TUPLE` the pointer may be `NULL` */ \
    if (hv.v_any && --hvalue_header_(&hv)->rc == 0) hvalue_free(hv);

        EACH_HVALUE_TYPE(TYPE_CASE)

#undef TYPE_CASE
#undef _COPY
#undef _RC
    }
}

static HValue hstring_clone(const HString *);
static HValue hsubstr_clone(HSubstr);
static HValue hclosure_clone(const HClosure *);
static HValue hcons_clone(const HCons *);
static HValue htuple_clone(const HTuple *);
static HValue hnative_clone(const HNative *);
static HValue hbinding_clone(const HBinding *);

HValue hvalue_clone(const HValue *hv) {
    switch (hv->type) {
    case V_EMPTY: panic("value is empty");
#define TYPE_CASE(name, data_type, data, strategy) \
    case name: return FORWARD(strategy, data);
#define _COPY(data)            *hv
#define _RC(free_fn, clone_fn) _RC_, clone_fn
#define _RC_(clone_fn, data)   clone_fn(hv->data)

        EACH_HVALUE_TYPE(TYPE_CASE)

#undef TYPE_CASE
#undef _COPY
#undef _RC
#undef _RC_
    }
}

inline HValue hvalue_uniq(HValue hv) {
    if (hvalue_is_uniq(&hv)) return hv;

    HValue clone = hvalue_clone(&hv);
    hvalue_drop(hv);
    return clone;
}

inline bool hvalue_is_uniq(const HValue *hv) {
    switch (hv->type) {
    case V_EMPTY: panic("value is empty");
#define TYPE_CASE(name, data_type, data, strategy) \
    case name: return strategy;
#define _COPY true
#define _RC(...)                                                     \
    /* for some RC types like `V_TUPLE` the pointer may be `NULL` */ \
    !hv->v_any || hvalue_header_(hv)->rc <= 1

        EACH_HVALUE_TYPE(TYPE_CASE)

#undef TYPE_CASE
#undef _COPY
#undef _RC
    }
}

static bool hcons_eq(const HCons *, const HCons *);
static bool htuple_eq(const HTuple *, const HTuple *);
static bool hnative_eq(const HNative *, const HValue *);

bool hvalue_eq(const HValue *a, const HValue *b) {
    // TODO: int <-> float equality
    switch (a->type) {
    case V_EMPTY: panic("value is empty");
    case V_INT: return b->type == V_INT && a->v_int == b->v_int;
    case V_FLOAT: return b->type == V_FLOAT && a->v_float == b->v_float;
    case V_STRING:
    case V_SUBSTR:
        return (b->type == V_STRING || b->type == V_SUBSTR)
            && string_eq(hvalue_string_get(a), hvalue_string_get(b));
    case V_CONS: return b->type == V_CONS && hcons_eq(a->v_cons, b->v_cons);
    case V_TUPLE: return b->type == V_TUPLE && htuple_eq(a->v_tuple, b->v_tuple);
    case V_NATIVE: return hnative_eq(a->v_native, b);
    case V_CLOSURE:
    case V_BINDING: return false;
    case V_TRUE:
    case V_FALSE:
    case V_NIL: return a->type == b->type;
    }
}

static void hclosure_print_repr(const HClosure *, Buffer *, const Machine *);
static void hcons_print_repr(const HCons *, Buffer *, const Machine *);
static void htuple_print_repr(const HTuple *, Buffer *, const Machine *);
static void hnative_print_repr(const HNative *, Buffer *, const Machine *);
static void hbinding_print_repr(const HBinding *, Buffer *, const Machine *);

void hvalue_print_repr(const HValue *hv, Buffer *b, const Machine *m) {
    switch (hv->type) {
    case V_EMPTY: panic("value is empty");
    case V_INT: buffer_printf(b, "%" PRIu32, hv->v_int); break;
    case V_FLOAT: buffer_printf(b, "%lf", hv->v_float); break;
    case V_STRING:
    case V_SUBSTR: buffer_print_string_literal(b, hvalue_string_get(hv)); break;
    case V_CLOSURE: hclosure_print_repr(hv->v_closure, b, m); break;
    case V_TRUE: buffer_puts(b, STRING("true")); break;
    case V_FALSE: buffer_puts(b, STRING("false")); break;
    case V_NIL: buffer_puts(b, STRING("[]")); break;
    case V_CONS: hcons_print_repr(hv->v_cons, b, m); break;
    case V_TUPLE: htuple_print_repr(hv->v_tuple, b, m); break;
    case V_NATIVE: hnative_print_repr(hv->v_native, b, m); break;
    case V_BINDING: hbinding_print_repr(hv->v_binding, b, m); break;
    }
}

static HValue hvalue_cons_bind(HValue, HValue, Machine *);
static HValue hvalue_native_bind(HValue, HValue, Machine *);
static HValue hvalue_binding_bind(HValue, HValue, Machine *);

HValue hvalue_bind(HValue effect, HValue then, Machine *m) {
    switch (effect.type) {
    case V_NIL: return effect;
    case V_CONS: return hvalue_cons_bind(effect, then, m);
    case V_NATIVE: return hvalue_native_bind(effect, then, m);
    case V_BINDING: return hvalue_binding_bind(effect, then, m);
    default: panic("%s does not support monadic binding", hvalue_type_name(effect.type));
    }
}

static HValue hnative_yield(const HNative *, const HValue *, Machine *);
static HValue hbinding_yield(const HBinding *, const HValue *, Machine *);

static bool hvalue_yield_impl(const HValue *hv, const HValue *then, Machine *m, HValue *result) {
    HValue tmp;
    if (!result) result = &tmp;

    switch (hv->type) {
    case V_NATIVE: *result = hnative_yield(hv->v_native, then, m); return true;
    case V_BINDING: *result = hbinding_yield(hv->v_binding, then, m); return true;
    default: return false;
    }
}

bool hvalue_yield(const HValue *hv, Machine *m, HValue *result) {
    return hvalue_yield_impl(hv, NULL, m, result);
}

inline HValue hvalue_make(hvalue_type type) {
    switch (type) {
    case V_FALSE:
    case V_TRUE:
    case V_NIL: return (HValue){ .type = type };
    default: panic("`hvalue_make` used with non-primitive type %s", hvalue_type_name(type));
    }
}

inline HValue hvalue_make_unit(void) {
    return (HValue){ .type = V_TUPLE, .v_tuple = NULL };
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
    switch (hv->type) {
    case V_STRING: return (string){ .data = hv->v_string->data, .len = hv->v_string->len };
    case V_SUBSTR:
        return (string){
            .data = hv->v_substr.string->data + hv->v_substr.offset,
            .len = hv->v_substr.len,
        };
    default: panic("value of type %s is not a string", hvalue_type_name(hv->type));
    }
}

HValue hvalue_make_substr(HValue hv, size_t offset, size_t len) {
    HSubstr substr = { 0 };

    switch (hv.type) {
    case V_STRING:
        substr.string = hv.v_string;
        substr.len = hv.v_string->len;
        break;
    case V_SUBSTR: substr = hv.v_substr; break;
    default: panic("cannot make a substring of %s", hvalue_type_name(hv.type));
    }

    offset = minsz(offset, substr.len);
    substr.offset += offset;
    substr.len = minsz(substr.len - offset, len);

    return (HValue){ .type = V_SUBSTR, .v_substr = substr };
}

static void hsubstr_free(HSubstr substr) {
    free((void *)substr.string);
}

static HValue hsubstr_clone(HSubstr substr) {
    return hvalue_make_substr(hstring_clone(substr.string), substr.offset, substr.len);
}

struct HClosure {
    hvalue_header header;
    uint32_t      fnindex;
    /*
     * Total number of required arguments (captures + args). This could
     * in theory be looked up in the metadata by `fnindex`, but it's easier to
     * have it directly stored as part of the object. This way e.g. cloning
     * doesn't require `Machine`
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
    free(closure);
}

static HValue hclosure_clone(const HClosure *closure) {
    HValue clone = hvalue_make_closure(closure->fnindex, closure->argc);

    clone.v_closure->args_len = closure->args_len;
    for (uint8_t i = 0; i < closure->args_len; i++)
        clone.v_closure->args[i] = hvalue_ref(&closure->args[i]);

    return clone;
}

static void hclosure_print_repr(const HClosure *closure, Buffer *b, const Machine *m) {
    string name = machine_func_name(m, closure->fnindex);

    buffer_printf(b, "(" F_STRING, FA_STRING(name));
    for (uint8_t i = 0; i < closure->args_len; i++) {
        buffer_putc(b, ' ');
        hvalue_print_repr(&closure->args[i], b, m);
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
    free(cons);
}

static HValue hcons_clone(const HCons *cons) {
    return hvalue_make_cons(hvalue_ref(&cons->head), hvalue_ref(&cons->tail));
}

static bool hcons_eq(const HCons *a, const HCons *b) {
    return hvalue_eq(&a->head, &b->head) && hvalue_eq(&a->tail, &b->tail);
}

static void hcons_print_repr(const HCons *cons, Buffer *b, const Machine *m) {
    buffer_putc(b, '[');

    while (1) {
        hvalue_print_repr(&cons->head, b, m);

        switch (cons->tail.type) {
        case V_CONS:
            cons = cons->tail.v_cons;
            buffer_puts(b, STRING(", "));
            break;
        case V_NIL: buffer_putc(b, ']'); return;
        default:
            hvalue_print_repr(&cons->tail, b, m);
            buffer_puts(b, STRING(" (malformed list)]"));
            return;
        }
    }
}

static HValue hvalue_cons_bind(HValue hv, HValue then, Machine *m) {
    HValue head, tail;
    hvalue_uncons(hv, &head, &tail);

    HValue first = machine_call(m, hvalue_ref(&then), head);
    HValue rest = hvalue_bind(tail, then, m);

    return hvalue_list_concat(first, rest);
}

inline bool hvalue_is_list(const HValue *hv) {
    return hv->type == V_NIL || hv->type == V_CONS;
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
    if (!tuple) return;

    for (uint8_t i = 0; i < tuple->len; i++) hvalue_drop(tuple->data[i]);
    free(tuple);
}

static HValue htuple_clone(const HTuple *tuple) {
    if (!tuple) return (HValue){ .type = V_TUPLE, .v_tuple = NULL };

    HTupleBuilder clone = htuple_begin(tuple->len);
    for (uint16_t i = 0; i < tuple->len; i++) htuple_put(&clone, hvalue_ref(&tuple->data[i]));
    return htuple_end(clone);
}

static bool htuple_eq(const HTuple *a, const HTuple *b) {
    if (a->len != b->len) return false;

    for (uint16_t i = 0; i < a->len; i++)
        if (!hvalue_eq(&a->data[i], &b->data[i])) return false;

    return true;
}

static void htuple_print_repr(const HTuple *tuple, Buffer *b, const Machine *ctx) {
    buffer_putc(b, '(');

    if (tuple) {
        for (uint16_t i = 0; i < tuple->len; i++) {
            if (i > 0) buffer_puts(b, STRING(", "));
            hvalue_print_repr(&tuple->data[i], b, ctx);
        }

        if (tuple->len == 1) buffer_putc(b, ',');
    }

    buffer_putc(b, ')');
}

HTupleBuilder htuple_begin(uint16_t len) {
    if (len == 0) return (HTupleBuilder){ .tuple = NULL, .len = 0 };

    HTuple *tuple = malloc(sizeof(HTuple) + sizeof(HValue) * len);
    tuple->header.rc = 1;
    tuple->len = len;
    return (HTupleBuilder){ .tuple = tuple, .len = 0 };
}

void htuple_put(HTupleBuilder *tb, HValue item) {
    debug_assert(tb->tuple);
    debug_assert(tb->len < tb->tuple->len);
    tb->tuple->data[tb->len++] = item;
}

HValue htuple_end(HTupleBuilder tb) {
    if (tb.tuple) debug_assert(tb.len == tb.tuple->len);
    return (HValue){ .type = V_TUPLE, .v_tuple = tb.tuple };
}

uint16_t hvalue_tuple_len(const HValue *hv) {
    const HTuple *tuple;
    hvalue_expect(hvalue_get_tuple, hv, &tuple);
    return tuple ? tuple->len : 0;
}

HValue hvalue_tuple_get(const HValue *hv, uint16_t index) {
    const HTuple *tuple;
    hvalue_expect(hvalue_get_tuple, hv, &tuple);
    debug_assert(tuple);
    debug_assert(index < tuple->len);
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
    free(native);
}

static HValue hnative_clone(const HNative *native) {
    void  *data_clone = native->meta->clone ? native->meta->clone(native->data) : native->data;
    HValue clone = hvalue_make_native(native->meta, data_clone);

    for (size_t i = 0; i < native->args_len; i++)
        clone.v_native->args[i] = hvalue_ref(&native->args[i]);

    return clone;
}

static bool hnative_eq(const HNative *a, const HValue *b) {
    // TODO: Implementation-defined
    (void)a, (void)b;
    return false;
}

static void hnative_print_repr(const HNative *native, Buffer *b, const Machine *m) {
    if (native->meta->print_repr) {
        native->meta->print_repr(native->data, native->args, native->args_len, b, m);
        return;
    }

    buffer_printf(b, "(%s", native->meta->name);
    for (uint8_t i = 0; i < native->args_len; i++) {
        buffer_putc(b, ' ');
        hvalue_print_repr(&native->args[i], b, m);
    }
    buffer_putc(b, ')');
}

static HValue hvalue_native_bind(HValue hv, HValue then, Machine *m) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, &hv, &native);

    if (native->meta->flags & HNATIVE_GENERIC_EFFECT) return hvalue_make_binding(hv, then);

    if (!native->meta->bind)
        panic("native value `%s` does not support monadic binding", native->meta->name);
    HValue result = native->meta->bind(native->data, &then, m);

    hvalue_drop(hv);
    return result;
}

static HValue hnative_yield(const HNative *native, const HValue *then, Machine *m) {
    if (!native->meta->yield)
        panic("native value `%s` is not a monadic effect", native->meta->name);
    return native->meta->yield(native->data, then, m);
}

const char *hvalue_native_name(const HValue *hv) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, hv, &native);
    return native->meta->name;
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

HValue hvalue_native_call(const HValue *hv, Machine *m) {
    const HNative *native;
    hvalue_expect(hvalue_get_native, hv, &native);
    debug_assert(native->args_len == native->meta->argc);
    debug_assert(native->meta->call);

    return native->meta->call(native->data, native->args, m);
}

struct HBinding {
    hvalue_header header;
    HValue        effect;
    HValue        then;
};

HValue hvalue_make_binding(HValue effect, HValue then) {
    HBinding *binding = malloc(sizeof(HBinding));
    binding->header.rc = 1;
    binding->effect = effect;
    binding->then = then;

    return (HValue){ .type = V_BINDING, .v_binding = binding };
}

static void hbinding_free(HBinding *binding) {
    hvalue_drop(binding->effect);
    hvalue_drop(binding->then);
    free(binding);
}

static HValue hbinding_clone(const HBinding *binding) {
    return hvalue_make_binding(hvalue_ref(&binding->effect), hvalue_ref(&binding->then));
}

static void hbinding_print_repr(const HBinding *binding, Buffer *b, const Machine *m) {
    buffer_puts(b, STRING("<binding "));
    hvalue_print_repr(&binding->effect, b, m);
    buffer_putc(b, ' ');
    hvalue_print_repr(&binding->then, b, m);
    buffer_putc(b, '>');
}

static HValue hbinding_yield(const HBinding *binding, const HValue *then, Machine *m) {
    HValue result;
    debug_assert(hvalue_yield_impl(&binding->effect, &binding->then, m, &result));
    if (hvalue_yield_impl(&result, then, m, &result)) return result;
    return then ? machine_call(m, hvalue_ref(then), result) : result;
}

static HValue hvalue_binding_bind(HValue hv, HValue then, Machine *m) {
    (void)m;
    return hvalue_make_binding(hv, then);
}

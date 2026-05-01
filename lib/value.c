#include "value.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

const char *hvalue_type_name(hvalue_type type) {
    RETURN_ENUM_NAME_V(hvalue_type, type, EACH_HVALUE_TYPE);
}

bool hvalue_is_rc(const HValue *hv) {
    switch (hv->type) {
#define TYPE_CASE(name, data_type, data_name, is_rc) \
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

void hvalue_drop(HValue hv) {
    if (hvalue_is_rc(&hv)) {
        if (!--hvalue_header_(&hv)->rc) {
            if (hv.type == V_NATIVE && hv.v_native->meta->free)
                hv.v_native->meta->free(hv.v_native->data);

            free(hv.v_any);
        }
    }
}

static HValue hvalue_closure_clone(const HValue *hv) {
    const HClosure *closure;
    debug_assert(hvalue_get_closure(hv, &closure));

    HValue clone = hvalue_make_closure(closure->fnindex, closure->argc);
    for (uint8_t i = 0; i < closure->args_len; i++)
        clone.v_closure->args[i] = hvalue_ref(&closure->args[i]);

    return clone;
}

static HValue hvalue_cons_clone(const HValue *hv) {
    const HCons *cons;
    debug_assert(hvalue_get_cons(hv, &cons));
    return hvalue_make_cons(hvalue_ref(&cons->head), hvalue_ref(&cons->tail));
}

static HValue hvalue_tuple_clone(const HValue *hv) {
    const HTuple *tuple;
    debug_assert(hvalue_get_tuple(hv, &tuple));

    HTupleBuilder clone = htuple_begin(tuple->len);
    for (uint16_t i = 0; i < tuple->len; i++) htuple_put(&clone, hvalue_ref(&tuple->data[i]));
    return htuple_end(clone);
}

static HValue hvalue_native_clone(const HValue *hv) {
    const HNative *native;
    debug_assert(hvalue_get_native(hv, &native));

    void  *data_clone = native->meta->clone ? native->meta->clone(native->data) : native->data;
    HValue clone = hvalue_make_native(native->meta, data_clone);

    for (size_t i = 0; i < native->args_len; i++)
        clone.v_native->args[i] = hvalue_ref(&native->args[i]);

    return clone;
}

HValue hvalue_clone(const HValue *hv) {
    switch (hv->type) {
    case V_STRING:
        return hvalue_make_string((string){
            .data = hv->v_string->data,
            .len = hv->v_string->len,
        });
    case V_CLOSURE: return hvalue_closure_clone(hv);
    case V_CONS: return hvalue_cons_clone(hv);
    case V_TUPLE: return hvalue_tuple_clone(hv);
    case V_NATIVE: return hvalue_native_clone(hv);
    default: debug_assert(!hvalue_is_rc(hv)); return *hv;
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

inline HValue hvalue_make(hvalue_type type) {
    switch (type) {
    case V_FALSE:
    case V_TRUE:
    case V_NIL: return (HValue){ .type = V_INT };
    default: panic("`hvalue_make` used with non-primitive type %s", hvalue_type_name(type));
    }
}

inline HValue hvalue_make_bool(bool b) {
    return hvalue_make(b ? V_TRUE : V_FALSE);
}

inline HValue hvalue_make_int(int64_t value) {
    return (HValue){ .type = V_INT, .v_int = value };
}

HValue hvalue_make_string(string value) {
    HString *data = malloc(sizeof(HString) + value.len);
    data->header.rc = 1;
    data->len = value.len;
    memcpy((void *)data->data, value.data, value.len);

    return (HValue){ .type = V_STRING, .v_string = data };
}

HValue hvalue_make_closure(uint32_t fnindex, uint8_t argc) {
    HClosure *data = malloc(sizeof(HClosure) + sizeof(HValue) * argc);
    data->header.rc = 1;
    data->fnindex = fnindex;
    data->argc = argc;
    data->args_len = 0;

    return (HValue){ .type = V_CLOSURE, .v_closure = data };
}

HValue hvalue_make_cons(HValue head, HValue tail) {
    HCons *data = malloc(sizeof(HCons));
    data->header.rc = 1;
    data->head = head;
    data->tail = tail;

    return (HValue){ .type = V_CONS, .v_cons = data };
}

HValue hvalue_make_native(const hnative_meta *meta, void *data) {
    HNative *native = malloc(sizeof(HNative) + sizeof(HValue) * meta->argc);
    native->header.rc = 1;
    native->meta = meta;
    native->data = data;
    native->args_len = 0;

    return (HValue){ .type = V_NATIVE, .v_native = native };
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

string hvalue_string_get(const HValue *hv) {
    const HString *str;
    debug_assert(hvalue_get_string(hv, &str));

    return (string){ .data = str->data, .len = str->len };
}

void hvalue_closure_put_arg_mut(const HValue *hv, HValue arg) {
    const HClosure *closure;
    debug_assert(hvalue_is_uniq(hv));
    debug_assert(hvalue_get_closure(hv, &closure));

    // SAFETY: Ensured unique, safe to mutate
    HClosure *closure_mut = (HClosure *)closure;
    closure_mut->args[closure_mut->args_len++] = arg;
}

bool hvalue_closure_take_arg_mut(const HValue *hv, HValue *arg) {
    const HClosure *closure;
    debug_assert(hvalue_is_uniq(hv));
    debug_assert(hvalue_get_closure(hv, &closure));

    if (closure->args_len == 0) return false;

    // SAFETY: Ensured unique, safe to mutate
    HClosure *closure_mut = (HClosure *)closure;
    *arg = closure_mut->args[--closure_mut->args_len];
    return true;
}

void hvalue_native_put_arg_mut(const HValue *hv, HValue arg) {
    const HNative *native;
    debug_assert(hvalue_is_uniq(hv));
    debug_assert(hvalue_get_native(hv, &native));

    debug_assert(native->args_len < native->meta->argc);

    // SAFETY: Ensured unique, safe to mutate
    HNative *native_mut = (HNative *)native;
    native_mut->args[native_mut->args_len++] = arg;
}

HValue hvalue_native_call(const HValue *hv) {
    const HNative *native;
    debug_assert(hvalue_get_native(hv, &native));
    debug_assert(native->args_len == native->meta->argc);
    debug_assert(native->meta->call);

    return native->meta->call(native->data, native->args);
}

HTupleBuilder htuple_begin(uint16_t len) {
    HTuple *tuple = malloc(sizeof(HTuple) + sizeof(HValue) * len);
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

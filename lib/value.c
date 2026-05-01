#include "value.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

const char *hvalue_type_name(hvalue_type type) {
    RETURN_ENUM_NAME_V(hvalue_type, type, EACH_HVALUE_TYPE);
}

inline bool hvalue_is_rc(const HValue *hv) {
    switch (hv->type) {
#define TYPE_CASE(name, data_type, data_name, is_rc) \
    case name: return is_rc;

        EACH_HVALUE_TYPE(TYPE_CASE)
#undef TYPE_CASE
    }
}

inline HValue hvalue_ref(const HValue *hv) {
    if (hvalue_is_rc(hv)) ((hvalue_header *)&hv)->rc++;
    return *hv;
}

inline void hvalue_drop(HValue hv) {
    if (hvalue_is_rc(&hv)) {
        if (!--((hvalue_header *)&hv)->rc) free(hv.data.v_nil);
    }
}

HValue hvalue_clone(const HValue *hv) {
    switch (hv->type) {
    case V_STRING:
        return hvalue_make_string((string){
            .data = hv->data.v_string->data,
            .len = hv->data.v_string->len,
        });
    case V_CLOSURE: panic("todo"); // TODO
    case V_CONS: panic("todo");    // TODO
    case V_TUPLE: panic("todo");   // TODO
    default: return *hv;
    }
}

inline HValue hvalue_uniq(HValue hv) {
    if (hvalue_is_uniq(&hv)) return hv;

    HValue clone = hvalue_clone(&hv);
    hvalue_drop(hv);
    return clone;
}

inline bool hvalue_is_uniq(const HValue *hv) {
    return !hvalue_is_rc(hv) || ((hvalue_header *)hv)->rc <= 1;
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
    return (HValue){ .type = V_INT, .data.v_int = value };
}

HValue hvalue_make_string(string value) {
    HString *data = malloc(sizeof(HString) + value.len);
    data->header.rc = 1;
    data->len = value.len;
    memcpy((void *)data->data, value.data, value.len);

    return (HValue){ .type = V_STRING, .data.v_string = data };
}

HValue hvalue_make_closure(uint32_t fnindex, uint8_t args) {
    HClosure *data = malloc(sizeof(HClosure) + sizeof(HValue) * args);
    data->header.rc = 1;
    data->fnindex = fnindex;
    data->args_len = 0;

    return (HValue){ .type = V_CLOSURE, .data.v_closure = data };
}

HValue hvalue_make_cons(HValue head, HValue tail) {
    HCons *data = malloc(sizeof(HCons));
    data->header.rc = 1;
    data->head = head;
    data->tail = tail;

    return (HValue){ .type = V_CONS, .data.v_cons = data };
}

#define HVALUE_GET(h, t, d, v)  \
    do {                        \
        if ((h)->type == (t)) { \
            *(v) = (h)->data.d; \
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

#undef HVALUE_GET

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
    return (HValue){ .type = V_TUPLE, .data.v_tuple = tb.tuple };
}

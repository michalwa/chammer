#ifndef HAMMER_VALUE_H_
#define HAMMER_VALUE_H_

#include <inttypes.h>

#include "buffer.h"
#include "machine.h"
#include "string.h"

#define EACH_HVALUE_TYPE(_)                                                  \
    /* _(name, data_type, data, is_rc, free, clone) */                       \
    _(V_INT, h_int, v_int, false, (void), HVALUE_COPY)                       \
    _(V_FLOAT, h_float, v_float, false, (void), HVALUE_COPY)                 \
    _(V_STRING, HString *, v_string, true, (void), hstring_clone)            \
    _(V_CLOSURE, HClosure *, v_closure, true, hclosure_free, hclosure_clone) \
    _(V_BINDING, HBinding *, v_binding, true, hbinding_free, hbinding_clone) \
    _(V_TRUE, h_unit, v_true, false, (void), HVALUE_COPY)                    \
    _(V_FALSE, h_unit, v_false, false, (void), HVALUE_COPY)                  \
    _(V_NIL, h_unit, v_nil, false, (void), HVALUE_COPY)                      \
    _(V_CONS, HCons *, v_cons, true, hcons_free, hcons_clone)                \
    _(V_TUPLE, HTuple *, v_tuple, true, htuple_free, htuple_clone)           \
    _(V_NATIVE, HNative *, v_native, true, hnative_free, hnative_clone)

#define ENUM_MEMBER(name, ...) name,
typedef enum { EACH_HVALUE_TYPE(ENUM_MEMBER) } hvalue_type;
#undef ENUM_MEMBER

/*
 * Placeholder for consistency, does not point to any data
 */
typedef void           *h_unit;
typedef int64_t         h_int;
typedef double          h_float;
typedef struct HString  HString;
typedef struct HClosure HClosure;
typedef struct HBinding HBinding;
typedef struct HCons    HCons;
typedef struct HTuple   HTuple;
typedef struct HNative  HNative;

/*
 * A Hammer value
 *
 * This is a shared reference type and must not be copied. Use `hvalue_ref` to
 * copy a reference and `hvalue_drop` to release it instead. Functions dealing
 * with `HValue` by value are assumed to transfer ownership, whereas
 * `const HValue *` is assumed to be a non-owning pointer unless specified
 * otherwise.
 *
 * Values should generally not be mutated, but some types like `V_CLOSURE` or
 * `V_NATIVE` do rely on mutation to avoid excessive clones. An `HValue` should
 * only be mutated if it holds a unique/exclusive reference to its allocation.
 * To ensure a value is unique or otherwise obtain a unique clone, use
 * `hvalue_uniq`.
 */
// NOTE: Struct declared with a name to allow forward definitions to resolve
// circular dependencies
typedef struct HValue {
    hvalue_type type;
    union {
        void *v_any;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-semi"
#define UNION_MEMBER(name, data_type, data, ...) data_type data;
        EACH_HVALUE_TYPE(UNION_MEMBER)
#undef UNION_MEMBER
#pragma clang diagnostic pop
    };
} HValue;

/*
 * All heap-allocated structs must include the header as the first field
 */
typedef struct hvalue_header hvalue_header;

typedef struct {
    HTuple  *tuple;
    uint16_t len;
} HTupleBuilder;

typedef struct {
    const char *name;
    size_t      argc;
    /*
     * Optional
     */
    HValue      (*call)(const void *data, const HValue *args, Machine *);
    /*
     * Optional
     */
    HValue      (*yield)(const void *data, const HValue *then, Machine *);
    /*
     * Optional
     */
    void        (*free)(void *);
    /*
     * Optional: if not specified, pointer will be copied
     */
    void       *(*clone)(const void *);
    /*
     * Optional: defaults to `(name ...args)`
     */
    void        (*print_repr)(
        const void *data, const HValue *args, size_t argc, Buffer *, const Machine *
    );
} hnative_meta;

const char *hvalue_type_name(hvalue_type);

hvalue_header *hvalue_header_(const HValue *);

/*
 * Checks whether the value contains reference-counted allocations
 */
bool   hvalue_is_rc(const HValue *);
/*
 * Make a shared reference to the value by incrementing the reference count
 */
HValue hvalue_ref(const HValue *);
/*
 * Releases the shared reference by decrementing the reference count
 */
void   hvalue_drop(HValue);
/*
 * Creates a new unique value that is a 1-level deep clone of the given value,
 * i.e. as opposed to `hvalue_ref`, direct allocations are actually physically
 * copied, but any nested `HValues` are still shared.
 */
HValue hvalue_clone(const HValue *);
/*
 * Returns a value reference that is ensured to be unique, i.e. no other `HValue`
 * will point to the same data. Takes ownership of the original reference and
 * releases it in case of a clone, so there's no need to call `hvalue_drop` on
 * it.
 */
HValue hvalue_uniq(HValue);
/*
 * Check whether the value reference is the only reference pointing at its data.
 * For primitive/stack types without allocations, always returns `true`. This
 * guarantees that the value can be safely mutated in place.
 */
bool   hvalue_is_uniq(const HValue *);

void hvalue_print_repr(const HValue *, Buffer *, const Machine *);

/*
 * Returns a primitive unit value
 */
HValue hvalue_make(hvalue_type);
HValue hvalue_make_unit(void);
/*
 * Shorthand for `hvalue_make(b ? V_TRUE : V_FALSE)`
 */
HValue hvalue_make_bool(bool);
HValue hvalue_make_int(int64_t);
HValue hvalue_make_string(string);
HValue hvalue_make_closure(uint32_t fnindex, uint8_t args);
HValue hvalue_make_cons(HValue head, HValue tail);
HValue hvalue_make_native(const hnative_meta *, void *);
HValue hvalue_make_binding(HValue lhs, HValue rhs);

bool hvalue_get_string(const HValue *, const HString **);
bool hvalue_get_closure(const HValue *, const HClosure **);
bool hvalue_get_cons(const HValue *, const HCons **);
bool hvalue_get_tuple(const HValue *, const HTuple **);
bool hvalue_get_native(const HValue *, const HNative **);
bool hvalue_get_binding(const HValue *, const HBinding **);

string hvalue_string_get(const HValue *);

/*
 * Returns the number of remaining arguments to be applied to the closure
 */
uint8_t  hvalue_closure_args_left(const HValue *);
uint32_t hvalue_closure_fnindex(const HValue *);
/*
 * Appends an argument to a closure. The closure must be unique (`hvalue_is_uniq`).
 * Takes ownership of the argument.
 *
 * NOTE: Does not check for bounds. Appending an argument to a closure that does not
 * expect any more arguments will result in overflow.
 */
void     hvalue_closure_put_arg_mut(const HValue *, HValue);
/*
 * Takes the last applied argument out of a closure. The closure must be unique
 * (`hvalue_is_uniq`). Returns `false` if no more arguments are left.
 */
bool     hvalue_closure_take_arg_mut(const HValue *, HValue *);

void   hvalue_uncons(HValue cons, HValue *head, HValue *tail);
HValue hvalue_list_concat(HValue, HValue);

HTupleBuilder htuple_begin(uint16_t len);
void          htuple_put(HTupleBuilder *, HValue);
HValue        htuple_end(HTupleBuilder);
uint16_t      hvalue_tuple_len(const HValue *);
HValue        hvalue_tuple_get(const HValue *, uint16_t);

/*
 * Same as `hvalue_closure_args_left` but for `V_NATIVE` values
 */
size_t hvalue_native_args_left(const HValue *);
/*
 * Same as `hvalue_closure_put_arg_mut` but for `V_NATIVE` values
 */
void   hvalue_native_put_arg_mut(const HValue *, HValue);
HValue hvalue_native_call(const HValue *, Machine *);
HValue hvalue_native_yield(const HValue *, const HValue *then, Machine *);

/*
 * Consumes the binding, executes the effect with the callback and returns the result
 */
HValue hvalue_binding_yield(HValue, Machine *);

#endif // HAMMER_VALUE_H_

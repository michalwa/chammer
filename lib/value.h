#ifndef HAMMER_VALUE_H_
#define HAMMER_VALUE_H_

#include <inttypes.h>

#include "buffer.h"
#include "bytecode.h"
#include "string.h"

#define EACH_HVALUE_TYPE(_)                    \
    /* _(name, data_type, data_name, is_rc) */ \
    _(V_INT, h_int, v_int, false)              \
    _(V_FLOAT, h_float, v_float, false)        \
    _(V_STRING, HString *, v_string, true)     \
    _(V_CLOSURE, HClosure *, v_closure, true)  \
    _(V_TRUE, h_unit, v_true, false)           \
    _(V_FALSE, h_unit, v_false, false)         \
    _(V_NIL, h_unit, v_nil, false)             \
    _(V_CONS, HCons *, v_cons, true)           \
    _(V_TUPLE, HTuple *, v_tuple, true)        \
    _(V_NATIVE, HNative *, v_native, true)

#define ENUM_MEMBER(name, data_type, data_name, is_rc) name,
typedef enum { EACH_HVALUE_TYPE(ENUM_MEMBER) } hvalue_type;
#undef ENUM_MEMBER

typedef void           *h_unit;
typedef int64_t         h_int;
typedef double          h_float;
typedef struct HString  HString;
typedef struct HClosure HClosure;
typedef struct HCons    HCons;
typedef struct HTuple   HTuple;
typedef struct HNative  HNative;

/*
 * A Hammer value
 *
 * This is a shared reference type and must not be copied. Use `hvalue_ref` to
 * copy a reference and `hvalue_drop` to release it instead. Functions dealing
 * with `HValue` by value are assumed to transfer ownership, whereas `HValue *`
 * is assumed to be a non-owning pointer.
 */
typedef struct {
    hvalue_type type;
    union {
        void *v_any;
#define UNION_MEMBER(name, data_type, data_name, is_rc) data_type data_name;
        EACH_HVALUE_TYPE(UNION_MEMBER)
#undef UNION_MEMBER
    };
} HValue;

/*
 * All heap-allocated structs must include the header as the first field
 */
typedef struct {
    uint32_t rc;
} hvalue_header;

struct HString {
    hvalue_header header;
    uint32_t      len;
    const char    data[];
};

struct HClosure {
    hvalue_header header;
    uint32_t      fnindex;
    /*
     * Total number of required arguments (captures + args)
     */
    uint8_t       argc;
    /*
     * Arguments applied so far, stored in `args`
     */
    uint8_t       args_len;
    HValue        args[];
};

struct HCons {
    hvalue_header header;
    HValue        head;
    HValue        tail;
};

struct HTuple {
    hvalue_header header;
    uint16_t      len;
    HValue        data[];
};

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
    HValue      (*call)(const void *data, const HValue *args);
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
        const void *data, const HValue *args, size_t argc, Buffer *, const program *
    );
} hnative_meta;

struct HNative {
    hvalue_header       header;
    const hnative_meta *meta;
    void               *data;
    size_t              args_len;
    HValue              args[];
};

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
 * Creates a new unique value that is a deep clone of the given value
 */
HValue hvalue_clone(const HValue *);
/*
 * Returns a value reference that is ensured to be unique, i.e. no other `HValue`
 * will point to the same data. Takes ownership of the original reference and
 * releases it in case of a clone.
 */
HValue hvalue_uniq(HValue);
/*
 * Check whether the value reference is the only reference pointing at its data.
 * For primitive/stack types without allocations, always returns `true`.
 */
bool   hvalue_is_uniq(const HValue *);

/*
 * Prints a string representation of the value to the buffer. `const program *`
 * is required for function metadata lookups
 */
void hvalue_print_repr(const HValue *, Buffer *, const program *);

HValue hvalue_make(hvalue_type);
HValue hvalue_make_bool(bool);
HValue hvalue_make_int(int64_t);
HValue hvalue_make_string(string);
HValue hvalue_make_closure(uint32_t fnindex, uint8_t args);
HValue hvalue_make_cons(HValue head, HValue tail);
HValue hvalue_make_native(const hnative_meta *, void *);

bool hvalue_get_string(const HValue *, const HString **);
bool hvalue_get_closure(const HValue *, const HClosure **);
bool hvalue_get_cons(const HValue *, const HCons **);
bool hvalue_get_tuple(const HValue *, const HTuple **);
bool hvalue_get_native(const HValue *, const HNative **);

string hvalue_string_get(const HValue *);

/*
 * Appends an argument to a closure. The closure must be unique (`hvalue_is_uniq`).
 * Takes ownership of the argument.
 *
 * NOTE: Does not check for bounds. Appending an argument to a closure that does not
 * expect any more arguments will result in overflow.
 */
void hvalue_closure_put_arg_mut(const HValue *, HValue);
/*
 * Takes the last applied argument out of a closure. The closure must be unique
 * (`hvalue_is_uniq`). Returns `false` if no more arguments are left.
 */
bool hvalue_closure_take_arg_mut(const HValue *, HValue *);

/*
 * Same as `hvalue_closure_put_arg_mut` but for `V_NATIVE` values
 */
void   hvalue_native_put_arg_mut(const HValue *, HValue);
HValue hvalue_native_call(const HValue *);

HTupleBuilder htuple_begin(uint16_t len);
void          htuple_put(HTupleBuilder *, HValue);
HValue        htuple_end(HTupleBuilder);

HValue hvalue_list_concat(HValue, HValue);

#endif // HAMMER_VALUE_H_

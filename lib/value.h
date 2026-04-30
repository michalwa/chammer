#ifndef HAMMER_VALUE_H_
#define HAMMER_VALUE_H_

#include <inttypes.h>

#include "string.h"

#define EACH_HVALUE_TYPE(_)             \
    _(V_INT, h_int, v_int)              \
    _(V_FLOAT, h_float, v_float)        \
    _(V_STRING, HString *, v_string)    \
    _(V_CLOSURE, HClosure *, v_closure) \
    _(V_TRUE, h_unit, v_true)           \
    _(V_FALSE, h_unit, v_false)         \
    _(V_NIL, h_unit, v_nil)             \
    _(V_CONS, HCons *, v_cons)          \
    _(V_TUPLE, HTuple *, v_tuple)

#define ENUM_MEMBER(name, data_type, data_name) name,
typedef enum { EACH_HVALUE_TYPE(ENUM_MEMBER) } hvalue_type;
#undef ENUM_MEMBER

typedef void           *h_unit;
typedef int64_t         h_int;
typedef double          h_float;
typedef struct HString  HString;
typedef struct HClosure HClosure;
typedef struct HCons    HCons;
typedef struct HTuple   HTuple;

/*
 * A Hammer value
 *
 * This is a shared reference type and must not be copied. Use `hvalue_ref` and
 * `hvalue_drop` instead.
 */
typedef struct {
    hvalue_type type;
    union {
#define UNION_MEMBER(name, data_type, data_name) data_type data_name;
        EACH_HVALUE_TYPE(UNION_MEMBER)
#undef UNION_MEMBER
    } data;
} HValue;

const char *hvalue_type_name(hvalue_type);

/*
 * Make a shared reference to the value by incrementing the reference count
 */
HValue hvalue_ref(HValue);
/*
 * Drops the shared reference by decrementing the reference count
 */
void   hvalue_drop(HValue);

HValue hvalue_make(hvalue_type);
HValue hvalue_make_int(int64_t);
HValue hvalue_make_string(string);

#endif // HAMMER_VALUE_H_

#include "value.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

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
    // TODO
};

struct HCons {
    hvalue_header header;
    HValue        head;
    HValue        tail;
};

struct HTuple {
    hvalue_header header;
    uint8_t       len;
    HValue        data[];
};

const char *hvalue_type_name(hvalue_type type) {
    RETURN_ENUM_NAME_V(hvalue_type, type, EACH_HVALUE_TYPE);
}

inline HValue hvalue_ref(HValue hv) {
    switch (hv.type) {
    case V_STRING:
    case V_CLOSURE:
    case V_CONS:
    case V_TUPLE: ((hvalue_header *)&hv)->rc++; return hv;
    default: return hv;
    }
}

void hvalue_drop(HValue hv) {
    switch (hv.type) {
    case V_STRING:
    case V_CLOSURE:
    case V_CONS:
    case V_TUPLE:
        if (!--((hvalue_header *)&hv)->rc) free(hv.data.v_nil);
        break;
    default: return;
    }
}

inline HValue hvalue_make(hvalue_type type) {
    switch (type) {
    case V_FALSE:
    case V_TRUE:
    case V_NIL: return (HValue){ .type = V_INT };
    default: panic("`hvalue_make` used with non-primitive type %s", hvalue_type_name(type));
    }
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

#include "value.h"

struct HString {
    uint32_t   rc;
    uint32_t   len;
    const char data[];
};

struct HClosure {
    uint32_t rc;
    uint32_t fnindex;
    // TODO
};

struct HCons {
    uint32_t rc;
    HValue   head;
    HValue   tail;
};

struct HTuple {
    uint32_t rc;
    uint8_t  len;
    HValue   data[];
};

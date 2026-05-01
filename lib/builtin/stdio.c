#include "stdio.h"

#include <stdlib.h>

#include "../utils.h"

static HValue print_call(const void *self, const HValue *argv) {
    debug_assert(!self);

    HValue *data = malloc(sizeof(HValue));
    *data = hvalue_ref(&argv[0]);
    return hvalue_make_native(&HNATIVE_META_PRINT, data);
}

static void print_free(void *self) {
    if (self) {
        hvalue_drop(*(HValue *)self);
        free(self);
    }
}

hnative_meta HNATIVE_META_PRINT = {
    .argc = 1,
    .call = print_call,
    .free = print_free,
};

HValue hnative_make_print(void) {
    return hvalue_make_native(&HNATIVE_META_PRINT, NULL);
}

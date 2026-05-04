#include "time.h"

#include <stdlib.h>
#include <time.h>

#include "../utils.h"

static HValue get_time_bind(const void *data, const HValue *then, Machine *m) {
    (void)data, (void)m;

    HValue *then_ref = malloc(sizeof(*then_ref));
    *then_ref = hvalue_ref(then);

    return hvalue_make_native(&HNATIVE_META_GET_TIME, then_ref);
}

static HValue get_time_yield(const void *self, Machine *m) {
    HValue *then = (HValue *)self;
    if (!then) return hvalue_make_unit();

    time_t     raw_time = time(NULL);
    struct tm *time_info = localtime(&raw_time);

    HTupleBuilder tb = htuple_begin(6);
    htuple_put(&tb, hvalue_make_int(time_info->tm_year + 1900));
    htuple_put(&tb, hvalue_make_int(time_info->tm_mon + 1));
    htuple_put(&tb, hvalue_make_int(time_info->tm_mday));
    htuple_put(&tb, hvalue_make_int(time_info->tm_hour));
    htuple_put(&tb, hvalue_make_int(time_info->tm_min));
    htuple_put(&tb, hvalue_make_int(time_info->tm_sec));
    HValue result = htuple_end(tb);

    return machine_call(m, hvalue_ref(then), result);
}

static void get_time_free(void *self) {
    if (self) {
        hvalue_drop(*(HValue *)self);
        free(self);
    }
}

static void *get_time_clone(const void *self) {
    if (!self) return NULL;

    HValue *clone = malloc(sizeof(HValue));
    *clone = hvalue_ref((HValue *)self);
    return clone;
}

hnative_meta HNATIVE_META_GET_TIME = {
    .name = "get_time",
    .argc = 0,
    .bind = get_time_bind,
    .yield = get_time_yield,
    .free = get_time_free,
    .clone = get_time_clone,
};

HValue hnative_make_get_time(void) {
    return hvalue_make_native(&HNATIVE_META_GET_TIME, NULL);
}

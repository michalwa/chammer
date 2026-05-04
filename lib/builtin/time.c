#include "time.h"

#include <time.h>

#include "../utils.h"

static HValue get_time_yield(const void *self, const HValue *then, Machine *m) {
    (void)self;

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

    return then ? machine_call(m, then, result) : result;
}

hnative_meta HNATIVE_META_GET_TIME = {
    .name = "get_time",
    .argc = 0,
    .yield = get_time_yield,
};

HValue hnative_make_get_time(void) {
    return hvalue_make_native(&HNATIVE_META_GET_TIME, NULL);
}

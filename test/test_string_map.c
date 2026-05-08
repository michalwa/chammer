#include "../lib/string_map.h"
#include "lib/test.h"

static int test_string_map(StringMap *m, Buffer *output_) {
    int value, old_value;

    value = 42;
    ASSERT(!string_map_put(m, STRING("foo"), &value, NULL));
    value = 123;
    ASSERT(!string_map_put(m, STRING("bar"), &value, NULL));

    ASSERT(string_map_get(m, STRING("foo"), &value));
    ASSERT_INT_EQ(value, 42);

    value = 1;
    ASSERT(string_map_put(m, STRING("bar"), &value, &old_value));
    ASSERT_INT_EQ(old_value, 123);

    ASSERT(string_map_get(m, STRING("bar"), &value));
    ASSERT_INT_EQ(value, 1);

    string_map_free(m);
    return TEST_OK;
}

TEST(string_map_single_bucket) {
    StringMap m;
    string_map_init_buckets(&m, char[4], 1);
    return test_string_map(&m, output_);
}

TEST(string_map_ten_buckets) {
    StringMap m;
    string_map_init_buckets(&m, char[4], 10);
    return test_string_map(&m, output_);
}

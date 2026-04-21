#include "../lib/string_pool.h"
#include "lib/test.h"

TEST(string_pool) {
    StringPool sp;
    string_pool_init(&sp);

    symbol foo = string_pool_intern(&sp, STRING("foo"));
    symbol bar = string_pool_intern(&sp, STRING("bar"));
    symbol foo2 = string_pool_intern(&sp, STRING("foo"));
    ASSERT(foo != bar);
    ASSERT_INT_EQ(foo, foo2);

    ASSERT_STRING_EQ(string_pool_get(&sp, foo), STRING("foo"));
    ASSERT_STRING_EQ(string_pool_get(&sp, bar), STRING("bar"));
    ASSERT_STRING_EQ(string_pool_get(&sp, foo2), STRING("foo"));

    string_pool_free(&sp);
    return TEST_OK;
}

#include "../lib/string_pool.h"
#include "lib/test.h"

TEST(string_pool) {
    StringPool sp;
    string_pool_init(&sp);

    symbol foo = string_pool_intern(&sp, STRING("foo"));
    symbol bar = string_pool_intern(&sp, STRING("bar"));
    symbol foo2 = string_pool_intern(&sp, STRING("foo"));
    ASSERT(foo != bar);
    ASSERT_INT_EQ((intptr_t)foo, (intptr_t)foo2);

    ASSERT_STRING_EQ(symbol_string(foo), STRING("foo"));
    ASSERT_STRING_EQ(symbol_string(bar), STRING("bar"));
    ASSERT_STRING_EQ(symbol_string(foo2), STRING("foo"));

    string_pool_free(&sp);
    return TEST_OK;
}

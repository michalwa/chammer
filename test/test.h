#ifndef TEST_H_
#define TEST_H_

#include <string.h>

#include "../lib/buffer.h"

#define TEST(name) int test_##name(Buffer *output)

#define TEST_OK   0
#define TEST_FAIL 1

#define TEST_PRINTF(...) buffer_printf(output, __VA_ARGS__)

#define ASSERT(expr)                                    \
    if (!(expr)) {                                      \
        TEST_PRINTF("Assertion failed: `" #expr "'\n"); \
        return TEST_FAIL;                               \
    }

#define ASSERT_EQ(format, a, b)                                                                 \
    if ((a) != (b)) {                                                                           \
        TEST_PRINTF(                                                                            \
            "Assertion failed: " #a " == " #b "\n   left = " format "\n  right = " format "\n", \
            (a), (b)                                                                            \
        );                                                                                      \
        return TEST_FAIL;                                                                       \
    }

#define ASSERT_INT_EQ(a, b) ASSERT_EQ("%d", (int)(a), (int)(b))

#define ASSERT_STRN_EQ(a, a_len, b, b_len) \
    ASSERT_INT_EQ(a_len, b_len);           \
    ASSERT(strncmp(a, b, a_len) == 0);

#endif // TEST_H_

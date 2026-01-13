#ifndef TEST_H_
#define TEST_H_

#include <string.h>

#include "../../lib/buffer.h"

#define RED(str)   "\033[0;31m" str "\033[0m"
#define GREEN(str) "\033[0;32m" str "\033[0m"

#define TEST(name) int test_##name(Buffer *output_)

#define TEST_OK   0
#define TEST_FAIL 1

#define TEST_PRINTF(...) buffer_printf(output_, __VA_ARGS__)

#define ASSERT_(expr, expr_str)                            \
    if (!(expr)) {                                         \
        TEST_PRINTF("Assertion failed: `" expr_str "'\n"); \
        return TEST_FAIL;                                  \
    }

#define ASSERT(expr) ASSERT_(expr, #expr)

#define ASSERT_EQ_(format, a, b, expr_str)                                                       \
    if ((a) != (b)) {                                                                            \
        TEST_PRINTF(                                                                             \
            "Assertion failed: " expr_str "\n   left = " format "\n  right = " format "\n", (a), \
            (b)                                                                                  \
        );                                                                                       \
        return TEST_FAIL;                                                                        \
    }

#define ASSERT_EQ(format, a, b) ASSERT_EQ_(format, a, b, #a " == " #b)
#define ASSERT_INT_EQ(a, b)     ASSERT_EQ_("%d", (int)(a), (int)(b), #a " == " #b)

#define ASSERT_STR_EQ(a, b)                                                        \
    if (strcmp(a, b) != 0) {                                                       \
        TEST_PRINTF(                                                               \
            "Assertion failed: strcmp(" #a ", " #b                                 \
            ", " ") == 0\n  ----- left ------\n%.*s\n  ----- right -----\n%.*s\n", \
            a, b                                                                   \
        );                                                                         \
        return TEST_FAIL;                                                          \
    }

#endif // TEST_H_

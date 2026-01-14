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

#define ASSERT_EQ(type, format, a, b, expr_str)                                               \
    {                                                                                         \
        type a_ = a;                                                                          \
        type b_ = b;                                                                          \
        if (a_ != b_) {                                                                       \
            TEST_PRINTF(                                                                      \
                "Assertion failed: " #a " == " #b "\n   left = " format "\n  right = " format \
                "\n",                                                                         \
                a_, b_                                                                        \
            );                                                                                \
            return TEST_FAIL;                                                                 \
        }                                                                                     \
    }

#define ASSERT_INT_EQ(a, b) ASSERT_EQ(int, "%d", (int)(a), (int)(b), #a " == " #b)

#define ASSERT_STR_EQ(a, b)                                                   \
    {                                                                         \
        const char *a_ = a;                                                   \
        const char *b_ = b;                                                   \
        if (strcmp(a_, b_) != 0) {                                            \
            TEST_PRINTF(                                                      \
                "Assertion failed: strcmp(" #a ", " #b                        \
                ") == 0\n  ----- left ------\n%s\n  ----- right -----\n%s\n", \
                a_, b_                                                        \
            );                                                                \
            return TEST_FAIL;                                                 \
        }                                                                     \
    }

#endif // TEST_H_

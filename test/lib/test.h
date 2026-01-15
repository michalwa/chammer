#ifndef TEST_H_
#define TEST_H_

#include <string.h> // strcmp

#include "../../lib/buffer.h" // buffer_printf

#define RED(str)   "\033[0;31m" str "\033[0m"
#define GREEN(str) "\033[0;32m" str "\033[0m"

#define TEST(name) int test_##name(Buffer *output_)

#define TEST_OK   0
#define TEST_FAIL 1

#define test_printf(...) buffer_printf(output_, __VA_ARGS__)

#define ASSERT_(expr, expr_str, ...)                                                       \
    do {                                                                                   \
        if (!(expr)) {                                                                     \
            test_printf(                                                                   \
                "Assertion failed: " expr_str "\n%s:%d\n", __VA_ARGS__, __FILE__, __LINE__ \
            );                                                                             \
            return TEST_FAIL;                                                              \
        }                                                                                  \
    } while (0)

#define ASSERT(expr) ASSERT_(expr, "%s", #expr)

#define ASSERT_EQ_(type, fmt, a, b)                                                    \
    do {                                                                               \
        type a_ = (a);                                                                 \
        type b_ = (b);                                                                 \
        ASSERT_(a_ == b_, #a " == " #b "\n   left = " fmt "\n  right = " fmt, a_, b_); \
    } while (0)

#define ASSERT_INT_EQ(a, b) ASSERT_EQ_(int, "%d", a, b)

#define ASSERT_ENUM_EQ(a, b, name_fn)                                                             \
    do {                                                                                          \
        int a_ = (a);                                                                             \
        int b_ = (b);                                                                             \
        ASSERT_(a_ == b_, #a " == " #b "\n   left = %s\n  right = %s", name_fn(a_), name_fn(b_)); \
    } while (0)

#define ASSERT_STR_EQ(a, b)                                                           \
    do {                                                                              \
        const char *a_ = (a);                                                         \
        const char *b_ = (b);                                                         \
        if (strcmp(a_, b_) != 0) {                                                    \
            test_printf("Assertion failed: strcmp(" #a ", " #b ") == 0\n   left = "); \
            buffer_print_c_string_literal(output_, a_);                               \
            test_printf("\n  right = ");                                              \
            buffer_print_c_string_literal(output_, b_);                               \
            test_printf("\n%s:%d\n", __FILE__, __LINE__);                             \
            return TEST_FAIL;                                                         \
        }                                                                             \
    } while (0)

void buffer_print_c_string_literal(Buffer *, const char *);

#endif // TEST_H_

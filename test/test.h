#ifndef TEST_H_
#define TEST_H_

#define TEST_OK   0
#define TEST_FAIL 1

#define TEST_PRINTF(...) buffer_printf(output, __VA_ARGS__)

#define ASSERT(expr)                                    \
    if (!(expr)) {                                      \
        TEST_PRINTF("Assertion failed: `" #expr "'\n"); \
        return TEST_FAIL;                               \
    }

#define ASSERT_EQ(a, b)                                                          \
    if ((a) != (b)) {                                                            \
        TEST_PRINTF("Assertion failed: `a == b'\n  a = " #a "\n  b = " #b "\n"); \
        return TEST_FAIL;                                                        \
    }

#endif // TEST_H_

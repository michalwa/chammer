#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/buffer.h"
#include "lib/test.h"

TEST(buffer_printf) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_printf(&b, "Hello, %s", "world");
    buffer_printf(&b, "!\n");

    ASSERT_INT_EQ(b.len, 14);
    ASSERT_INT_EQ(b.capacity, 16);
    ASSERT_STRN_EQ(b.data, b.len, "Hello, world!\n", 14);

    buffer_free(&b);
    return TEST_OK;
}

TEST(buffer_putc) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_putc(&b, '?');
    ASSERT_INT_EQ(b.len, 1);
    ASSERT_INT_EQ(b.capacity, 8);

    for (int i = 0; i < 20; i++) buffer_putc(&b, 'a' + i);

    ASSERT_INT_EQ(b.len, 21);
    ASSERT_INT_EQ(b.capacity, 32);
    ASSERT_STRN_EQ(b.data, b.len, "?abcdefghijklmnopqrst", 21);

    buffer_free(&b);
    return TEST_OK;
}

TEST(buffer_alloc) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_alloc(&b, 10);
    ASSERT_INT_EQ(b.len, 10);
    ASSERT_INT_EQ(b.capacity, 16);

    buffer_free(&b);
    return TEST_OK;
}

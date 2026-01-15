#include "../lib/buffer.h"
#include "lib/test.h"

TEST(buffer_putc) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_putc(&b, '?');
    ASSERT_INT_EQ(b.len, 1);
    ASSERT_INT_EQ(b.capacity, 8);
    ASSERT_INT_EQ(b.data[b.len], 0);

    for (int i = 0; i < 20; i++) buffer_putc(&b, 'a' + i);

    ASSERT_INT_EQ(b.len, 21);
    ASSERT_INT_EQ(b.capacity, 32);
    ASSERT_INT_EQ(b.data[b.len], 0);
    ASSERT_STR_EQ(b.data, "?abcdefghijklmnopqrst");

    buffer_free(&b);
    return TEST_OK;
}

TEST(buffer_printf) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_printf(&b, "Hello, %s", "world");
    buffer_printf(&b, "!\n");

    ASSERT_INT_EQ(b.len, 14);
    ASSERT_INT_EQ(b.capacity, 16);
    ASSERT_INT_EQ(b.data[b.len], 0);
    ASSERT_STR_EQ(b.data, "Hello, world!\n");

    buffer_printf(&b, "...");

    ASSERT_INT_EQ(b.len, 17);
    ASSERT_INT_EQ(b.capacity, 32);
    ASSERT_INT_EQ(b.data[b.len], 0);

    buffer_free(&b);
    return TEST_OK;
}

TEST(buffer_alloc) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_alloc(&b, 10);
    ASSERT_INT_EQ(b.len, 10);
    ASSERT_INT_EQ(b.capacity, 16);
    ASSERT_INT_EQ(b.data[b.len], 0);

    buffer_alloc(&b, 7);
    ASSERT_INT_EQ(b.len, 17);
    ASSERT_INT_EQ(b.capacity, 32);
    ASSERT_INT_EQ(b.data[b.len], 0);

    buffer_free(&b);
    return TEST_OK;
}

TEST(buffer_read_file) {
    Buffer b;
    buffer_init(&b);

    FILE *file = fopen("test/dummy.txt", "r");
    buffer_read_file(&b, file);
    fclose(file);

    ASSERT_INT_EQ(b.len, 5);
    ASSERT_INT_EQ(b.data[b.len], 0);
    ASSERT_STR_EQ(b.data, "hello");

    file = fopen("test/dummy.txt", "r");
    buffer_read_file(&b, file);
    fclose(file);

    ASSERT_INT_EQ(b.len, 10);
    ASSERT_INT_EQ(b.data[b.len], 0);
    ASSERT_STR_EQ(b.data, "hellohello");

    buffer_free(&b);
    return TEST_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib/buffer.h"
#include "lib/test.h"

TEST(buffer) {
    Buffer b;
    buffer_init_capacity(&b, 8);

    buffer_printf(&b, "Hello, %s", "world");
    buffer_printf(&b, "!\n");

    ASSERT_INT_EQ(b.len, 14);
    ASSERT_INT_EQ(b.capacity, 16);
    ASSERT(strncmp("Hello, world!\n", b.data, b.len) == 0);

    return TEST_OK;
}

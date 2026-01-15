#include "lib/test.h"

// TEST(this comment should be ignored)

TEST(buffer_print_c_string_literal) {
    Buffer buffer;
    buffer_init(&buffer);

    buffer_print_c_string_literal(&buffer, "hello\nworld");
    ASSERT_STR_EQ(buffer.data, "\"hello\\nworld\"");

    buffer_free(&buffer);
    return TEST_OK;
}

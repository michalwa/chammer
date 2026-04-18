#include "../lib/compiler.h"
#include "lib/test.h"

TEST(compile_string) {
    Buffer out;
    buffer_init(&out);

    size_t offset, len;

    compile_string(STRING("\"\""), &out, &offset, &len);
    ASSERT_STR_EQ(out.data, "");
    ASSERT_INT_EQ(offset, 0);
    ASSERT_INT_EQ(len, 0);
    buffer_clear(&out);

    compile_string(STRING("\"foo\""), &out, &offset, &len);
    ASSERT_STR_EQ(out.data, "foo");
    ASSERT_INT_EQ(offset, 0);
    ASSERT_INT_EQ(len, 3);

    compile_string(STRING("\" bar\""), &out, &offset, &len);
    ASSERT_STR_EQ(out.data, "foo bar");
    ASSERT_INT_EQ(offset, 3);
    ASSERT_INT_EQ(len, 4);
    buffer_clear(&out);

    compile_string(STRING("\"\\\"\\n\""), &out, &offset, &len);
    ASSERT_STR_EQ(out.data, "\"\n");
    ASSERT_INT_EQ(offset, 0);
    ASSERT_INT_EQ(len, 2);
    buffer_clear(&out);

    buffer_free(&out);

    return TEST_OK;
}

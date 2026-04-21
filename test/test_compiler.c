#include "../lib/compiler.h"
#include "lib/test.h"

TEST(compile_string) {
    Buffer out;
    buffer_init(&out);

    compile_string(STRING("\"\""), &out);
    ASSERT_STR_EQ(out.data, "");
    buffer_clear(&out);

    compile_string(STRING("\"foo\""), &out);
    ASSERT_STR_EQ(out.data, "foo");

    compile_string(STRING("\" bar\""), &out);
    ASSERT_STR_EQ(out.data, "foo bar");
    buffer_clear(&out);

    compile_string(STRING("\"\\\"\\n\""), &out);
    ASSERT_STR_EQ(out.data, "\"\n");
    buffer_clear(&out);

    buffer_free(&out);

    return TEST_OK;
}

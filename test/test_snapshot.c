#include "lib/snapshot.h"
#include "lib/test.h"

TEST(snapshot_diff) {
    Buffer output;
    buffer_init(&output);

    snapshot_diff(&output, "foo", "foo");
    ASSERT_STR_EQ(output.data, "  foo\n");

    buffer_clear(&output);
    snapshot_diff(&output, "foo\nbar", "foo\nbar");
    ASSERT_STR_EQ(output.data, "  foo\n  bar\n");

    buffer_clear(&output);
    snapshot_diff(&output, "foo", "bar");
    ASSERT_STR_EQ(output.data, RED("- foo\n") GREEN("+ bar\n"));

    buffer_free(&output);
    return TEST_OK;
}

TEST(snapshot_diff_empty_lines) {
    Buffer output;
    buffer_init(&output);

    snapshot_diff(&output, "foo\r\nbar", "foo\r\n\nbar");
    ASSERT_STR_EQ(output.data, "  foo\n" RED("- bar\n") GREEN("+ \n") GREEN("+ bar\n"));

    buffer_free(&output);
    return TEST_OK;
}

TEST(snapshot_diff_line_endings) {
    Buffer output;
    buffer_init(&output);

    snapshot_diff(&output, "foo\r\n", "foo\n");
    ASSERT_STR_EQ(output.data, "  foo\n");

    buffer_clear(&output);
    snapshot_diff(&output, "foo", "foo\n");
    ASSERT_STR_EQ(output.data, "  foo\n");

    buffer_clear(&output);
    snapshot_diff(&output, "foo\r\nbar", "foo\nbar");
    ASSERT_STR_EQ(output.data, "  foo\n  bar\n");

    buffer_free(&output);
    return TEST_OK;
}

#include <stdio.h>
#include <stdlib.h>

#include "../lib/buffer.h"
#include "../lib/lexer.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EXAMPLE_FILE_PATH "test/example.ham"

TEST(lexer_example) {
    FILE *f = fopen(EXAMPLE_FILE_PATH, "r");
    if (!f) {
        perror("Could not open `" EXAMPLE_FILE_PATH "': ");
        return TEST_FAIL;
    }

    Buffer input_buf;
    buffer_init(&input_buf);
    fread_to_buffer(f, &input_buf);
    buffer_putc(&input_buf, '\0');

    Buffer output_buf;
    buffer_init(&output_buf);

    token token;
    token_begin(&token, input_buf.data);

    while (token_next(&token) == LEX_OK) {
        loc loc = token_loc(token, input_buf.data);
        buffer_printf(
            &output_buf, F_TOKEN " at %d:%d\n", FA_TOKEN(token), loc.line + 1, loc.col + 1
        );
    }

    buffer_putc(&output_buf, '\0');

    SNAPSHOT("tokens_example", output_buf.data);

    buffer_free(&input_buf);
    buffer_free(&output_buf);

    return TEST_OK;
}

TEST(lexer_empty) {
    token token;
    token_begin(&token, "");

    ASSERT_INT_EQ(token_next(&token), LEX_NONE);

    return TEST_OK;
}

TEST(lexer_unclosed_string) {
    token token;
    token_begin(&token, "\"foo");

    ASSERT_INT_EQ(token_next(&token), LEX_EEOI);

    return TEST_OK;
}

TEST(lexer_unclosed_block_comment) {
    token token;
    token_begin(&token, "{- foo");

    ASSERT_INT_EQ(token_next(&token), LEX_EEOI);

    return TEST_OK;
}

TEST(lexer_unclosed_decimal) {
    token token;
    token_begin(&token, "1.");

    ASSERT_INT_EQ(token_next(&token), LEX_ENUM);

    return TEST_OK;
}

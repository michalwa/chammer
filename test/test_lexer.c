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

    Buffer buffer;
    buffer_init(&buffer);
    fread_to_buffer(f, &buffer);
    buffer_putc(&buffer, '\0');

    token token;
    token_begin(&token, buffer.data);

    SNAPSHOT("tokens_example", f, {
        while (token_next(&token) == LEX_OK) {
            loc loc = token_loc(token, buffer.data);
            fprintf(f, F_TOKEN " at %d:%d\n", FA_TOKEN(token), loc.line + 1, loc.col + 1);
        }
    });

    buffer_free(&buffer);

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

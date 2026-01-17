#include <stdio.h>

#include "../lib/buffer.h"
#include "../lib/lexer.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EXAMPLE_FILE_PATH "examples/html.ham"

TEST(lexer_full_example) {
    FILE *f = fopen(EXAMPLE_FILE_PATH, "r");
    if (!f) {
        perror("Could not open `" EXAMPLE_FILE_PATH "`: ");
        return TEST_FAIL;
    }

    Buffer input;
    buffer_init(&input);
    buffer_read_file(&input, f);
    fclose(f);

    Buffer output;
    buffer_init(&output);

    token token;
    token_begin(&token, input.data);

    while (token_next(&token, LEX_ALL) == LEX_OK) {
        loc loc = token_loc(token, input.data);
        buffer_printf(&output, F_TOKEN " at %d:%d\n", FA_TOKEN(token), loc.line + 1, loc.col + 1);
    }

    SNAPSHOT("lexer_full_example", output.data);

    buffer_free(&input);
    buffer_free(&output);

    return TEST_OK;
}

TEST(lexer_empty) {
    token token;
    token_begin(&token, "");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_NONE, lex_result_name);

    return TEST_OK;
}

TEST(lexer_empty_string_after_keyword) {
    token token;
    token_begin(&token, "then \"\"");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_OK, lex_result_name);
    ASSERT_ENUM_EQ(token.type, T_THEN, token_type_name);

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_OK, lex_result_name);
    ASSERT_ENUM_EQ(token.type, T_STRING, token_type_name);
    ASSERT_INT_EQ(token.len, 2);

    return TEST_OK;
}

TEST(lexer_unclosed_string) {
    token token;
    token_begin(&token, "\"foo");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_EEOI, lex_result_name);

    return TEST_OK;
}

TEST(lexer_unclosed_block_comment) {
    token token;
    token_begin(&token, "{- foo");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_EEOI, lex_result_name);

    return TEST_OK;
}

TEST(lexer_unclosed_decimal) {
    token token;
    token_begin(&token, "1.");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_ENUM, lex_result_name);

    return TEST_OK;
}

TEST(lexer_under) {
    token token;
    token_begin(&token, "_");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_OK, lex_result_name);
    ASSERT_ENUM_EQ(token.type, T_UNDER, token_type_name);

    return TEST_OK;
}

TEST(lexer_minus_and_rarrow) {
    token token;
    token_begin(&token, "- ->");

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_OK, lex_result_name);
    ASSERT_ENUM_EQ(token.type, T_OP, token_type_name);

    ASSERT_ENUM_EQ(token_next(&token, LEX_ALL), LEX_OK, lex_result_name);
    ASSERT_ENUM_EQ(token.type, T_RARROW, token_type_name);

    return TEST_OK;
}

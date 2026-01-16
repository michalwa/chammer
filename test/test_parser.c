#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EXAMPLE_FILE_PATH "test/example.ham"

TEST(parser_example) {
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

    Parser parser;
    parser_init(&parser);
    ASSERT_ENUM_EQ(parse_program(&parser, &token), PARSE_OK, parse_result_name);
    node_print(*parser.node, &output);
    SNAPSHOT("parser_example", output.data);

    buffer_free(&input);
    buffer_free(&output);

    return TEST_OK;
}

TEST(atoms) {
    token  t;
    Parser p;

    parser_init(&p);
    token_begin(&t, "1");
    ASSERT_ENUM_EQ(parse_int(&p, &t), PARSE_OK, parse_result_name);
    ASSERT_ENUM_EQ(p.node->type, N_INT, node_type_name);
    parser_free(&p);

    parser_init(&p);
    token_begin(&t, "foo");
    ASSERT_ENUM_EQ(parse_ident(&p, &t), PARSE_OK, parse_result_name);
    ASSERT_ENUM_EQ(p.node->type, N_IDENT, node_type_name);
    parser_free(&p);

    parser_init(&p);
    token_begin(&t, "1");
    ASSERT_ENUM_EQ(parse_ident(&p, &t), PARSE_ETOK, parse_result_name);
    ASSERT_ENUM_EQ(p.expected_token, T_IDENT, token_type_name);
    parser_free(&p);

    return TEST_OK;
}

TEST(parse_tuple_or_parens) {
    token t;
    token_begin(&t, "(foo, bar)");

    Parser p;
    parser_init(&p);

    Buffer output;
    buffer_init(&output);

    ASSERT_ENUM_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK, parse_result_name);
    node_print(*p.node, &output);
    SNAPSHOT("tuple_with_2_idents", output.data);

    parser_free(&p);
    buffer_free(&output);
    return TEST_OK;
}

TEST(parse_binary) {
    token t;
    // (((1 * 2) + (3 * 4)) >> (5 >> 6)) => (7 + (8 * 9)))
    token_begin(&t, "1 * 2 + 3 * 4 >> 5 >> 6 => 7 + 8 * 9");

    Parser p;
    parser_init(&p);
    parser_define_operator(&p, "*", 1, 100, ASSOC_LEFT);
    parser_define_operator(&p, "+", 1, 200, ASSOC_LEFT);
    parser_define_operator(&p, ">>", 2, 300, ASSOC_RIGHT);
    parser_define_operator(&p, "=>", 2, 400, ASSOC_RIGHT);

    Buffer output;
    buffer_init(&output);

    ASSERT_ENUM_EQ(parse_binary(&p, &t), PARSE_OK, parse_result_name);
    node_print(*p.node, &output);
    SNAPSHOT("binary", output.data);

    parser_free(&p);
    buffer_free(&output);
    return TEST_OK;
}

TEST(elaborate_expression) {
    token t;
    token_begin(&t, "(1, 2, \"foo\", foo, [42, bar, 3.14], (), [], if 1 then 2 else 3)");

    Parser p;
    parser_init(&p);

    Buffer output;
    buffer_init(&output);

    ASSERT_ENUM_EQ(parse_expr(&p, &t, EXPR_ALL), PARSE_OK, parse_result_name);
    node_print(*p.node, &output);
    SNAPSHOT("elaborate_expression", output.data);

    parser_free(&p);
    buffer_free(&output);
    return TEST_OK;
}

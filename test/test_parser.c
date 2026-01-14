#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

TEST(atoms) {
    token  t;
    Parser p;

    parser_init(&p);
    token_begin(&t, "1");
    ASSERT_INT_EQ(parse_int(&p, &t), PARSE_OK);
    ASSERT_INT_EQ(p.node->type, N_INT);
    parser_free(&p);

    parser_init(&p);
    token_begin(&t, "foo");
    ASSERT_INT_EQ(parse_ident(&p, &t), PARSE_OK);
    ASSERT_INT_EQ(p.node->type, N_IDENT);
    parser_free(&p);

    parser_init(&p);
    token_begin(&t, "1");
    ASSERT_INT_EQ(parse_ident(&p, &t), PARSE_ETOK);
    ASSERT_INT_EQ(p.expected_token, T_IDENT);
    parser_free(&p);

    return TEST_OK;
}

TEST(parse_tuple_or_parens) {
    token  t;
    Parser p;
    Buffer output;

    token_begin(&t, "(foo, bar)");
    parser_init(&p);
    buffer_init(&output);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);
    node_print(*p.node, &output);
    SNAPSHOT("tuple_with_2_idents", output.data);

    parser_free(&p);
    buffer_free(&output);

    return TEST_OK;
}

TEST(elaborate_expression) {
    token  t;
    Parser p;
    Buffer output;

    token_begin(&t, "(1, 2, \"foo\", foo, [42, bar, 3.14], (), [], if 1 then 2 else 3)");

    parser_init(&p);
    buffer_init(&output);

    ASSERT_INT_EQ(parse_expr(&p, &t, EXPR_ALL), PARSE_OK);
    node_print(*p.node, &output);
    SNAPSHOT("elaborate_expression", output.data);

    parser_free(&p);
    buffer_free(&output);

    return TEST_OK;
}

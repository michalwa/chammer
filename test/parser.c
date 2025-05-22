#include "../lib/parser.h"

#include "../lib/lexer.h"
#include "test.h"

TEST(parse_tuple_or_parens) {
    token t;
    token_begin(&t, "(foo, bar)");

    Parser p;
    parser_init(&p);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);

    const node *n = p.node;
    ASSERT_INT_EQ(n->type, N_TUPLE);

    n = n->first_child;
    ASSERT_INT_EQ(n->type, N_IDENT);
    ASSERT_INT_EQ(n->token.type, T_IDENT);
    ASSERT_STRN_EQ(n->token.str, n->token.len, "foo", 3);

    n = n->next_sibling;
    ASSERT_INT_EQ(n->type, N_IDENT);
    ASSERT_INT_EQ(n->token.type, T_IDENT);
    ASSERT_STRN_EQ(n->token.str, n->token.len, "bar", 3);

    return TEST_OK;
}

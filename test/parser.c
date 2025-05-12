#include "../lib/parser.h"

#include "../lib/lexer.h"
#include "test.h"

TEST(parse_tuple_or_parens) {
    token t;
    token_begin(&t, "(foo, bar)");

    Parser p;
    parser_init(&p);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);

    node_print(*p.node, stdout);

    return TEST_OK;
}

#include <stdio.h>

#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

TEST(parse_tuple_or_parens) {
    token t;
    token_begin(&t, "(foo, bar)");

    Parser p;
    parser_init(&p);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);
    SNAPSHOT("tuple_with_2_idents", f, node_print(*p.node, f));

    parser_free(&p);

    return TEST_OK;
}

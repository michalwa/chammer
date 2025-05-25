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

    Buffer output;
    buffer_init(&output);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);
    node_print(*p.node, &output);
    SNAPSHOT("tuple_with_2_idents", output.data);

    parser_free(&p);
    buffer_free(&output);

    return TEST_OK;
}

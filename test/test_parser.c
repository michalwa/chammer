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

    Buffer b;
    buffer_init(&b);

    ASSERT_INT_EQ(parse_tuple_or_parens(&p, &t), PARSE_OK);
    node_print(*p.node, &b);
    buffer_putc(&b, '\0');
    SNAPSHOT("tuple_with_2_idents", b.data);

    parser_free(&p);
    buffer_free(&b);

    return TEST_OK;
}

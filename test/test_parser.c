#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EXAMPLE_FILE_PATH "test/example.ham"

TEST(parser_full_example) {
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
    SNAPSHOT("parser_full_example", output.data);

    parser_free(&parser);
    buffer_free(&input);
    buffer_free(&output);

    return TEST_OK;
}

#define EACH_EXAMPLE(_, p, t)                                                          \
    /* _(name, parser, source) */                                                      \
    _("int", parse_int(p, t), "1")                                                     \
    _("ident", parse_ident(p, t), "foo")                                               \
    _("tuple_empty", parse_tuple_or_parens(p, t), "()")                                \
    _("not_a_tuple", parse_tuple_or_parens(p, t), "(foo)")                             \
    _("tuple_singleton", parse_tuple_or_parens(p, t), "(foo,)")                        \
    _("tuple_pair", parse_tuple_or_parens(p, t), "(foo, bar)")                         \
    _("tuple_trailing_comma", parse_tuple_or_parens(p, t), "(foo, bar,)")              \
    _("tuple_spread", parse_tuple_or_parens(p, t), "(foo, ...bar)")                    \
    _("list_empty", parse_list(p, t), "[]")                                            \
    _("list_singleton", parse_list(p, t), "[foo]")                                     \
    _("list_pair", parse_list(p, t), "[foo, bar]")                                     \
    _("list_trailing_comma", parse_list(p, t), "[foo, bar,]")                          \
    _("list_spread", parse_list(p, t), "[foo, ...bar]")                                \
    _("binary_precedence", parse_binary(p, t), "1 * 2 + 3 * 4 >> 5 >> 6 => 7 + 8 * 9") \
    _("elaborate_expr", parse_expr(p, t, EXPR_ALL),                                    \
      "(1, 2, \"foo\", foo, [42, bar, 3.14], (), [], if 1 then 2 else 3)")             \
    _("assign", parse_assign(p, t), "let x = 1;")                                      \
    _("dobind", parse_dobind(p, t), "let x <- 1;")                                     \
    _("tuple_singleton_unpack", parse_assign(p, t), "let (a,) = (1,);")                \
    _("tuple_pair_unpack", parse_assign(p, t), "let (a, b) = (1, 2);")                 \
    _("list_pair_unpack", parse_assign(p, t), "let [a, b] = [1, 2];")                  \
    _("list_tail", parse_assign(p, t), "let [a, ...] = [1, 2, 3];")                    \
    _("list_tail_named", parse_assign(p, t), "let [a, ...rest] = [1, 2, 3];")

static void define_example_operators(Parser *p) {
    parser_define_operator(p, "*", 1, 100, ASSOC_LEFT);
    parser_define_operator(p, "+", 1, 200, ASSOC_LEFT);
    parser_define_operator(p, ">>", 2, 300, ASSOC_RIGHT);
    parser_define_operator(p, "=>", 2, 400, ASSOC_RIGHT);
}

TEST(parser_examples) {
    token  t;
    Parser p;
    Buffer output;

    buffer_init(&output);
    parser_init(&p);
    define_example_operators(&p);

#define RUN(name, parse, source)                        \
    token_begin(&t, source);                            \
    ASSERT_ENUM_EQ(parse, PARSE_OK, parse_result_name); \
    node_print(*p.node, &output);                       \
    SNAPSHOT("parser_example_" name, output.data);      \
    parser_reset(&p);                                   \
    buffer_clear(&output);

    EACH_EXAMPLE(RUN, &p, &t)
#undef RUN

    parser_free(&p);
    buffer_free(&output);

    return TEST_OK;
}

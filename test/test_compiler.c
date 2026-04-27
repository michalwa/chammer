#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EACH_EXAMPLE(_)                                                                        \
    _("int", "42")                                                                             \
    _("string", "\"foo\"")                                                                     \
    _("tuple", "(1, 2, 3)")                                                                    \
    _("list", "[1, 2, 3]")                                                                     \
    _("list_spread", "[1, 2, ...[3, 4], 5]")                                                   \
    _("extern", "print 1")                                                                     \
    _("if", "if 1 then 2 else 3")                                                              \
    _("if_nested", "if 1 then if 2 then 3 else 4 else if 5 then 6 else 7")                     \
    _("unary", "!1")                                                                           \
    _("binary", "1 % 2")                                                                       \
    _("assign", "let x = 1; let y = 2; (x, y)")                                                \
    _("assign_nested", "let x = 1; let y = { let x = 2; x }; (x, y)")                          \
    _("assign_capture", "let x = 1; let y = { let z = 2; (x, z) }; y")                         \
    _("tuple_unpack", "let (x, y) = (1, 2); (x, y)")                                           \
    _("list_unpack", "let [x, y] = [1, 2]; (x, y)")                                            \
    _("list_unpack_tail", "let [x, ...] = [1, 2, 3]; x")                                       \
    _("list_unpack_tail_named", "let [x, ...xs] = [1, 2, 3]; (x, xs)")                         \
    _("function_unary", "let f x = (x,); f 1")                                                 \
    _("function_binary", "let f x y = (x, y); f 1 2")                                          \
    _("function_nested", "let f x = { let g x y = (x, y); g x x }; f 1")                       \
    _("function_tuple_unpack", "let f (x, y) = (y, x); f (1, 2)")                              \
    _("lambda", "let f = \\x y -> (x, y); f 1 2")                                              \
    _("lambda_tuple_unpack", "let f = \\(x, y) -> (y, x); f (1, 2)")                           \
    _(                                                                                       \
        "let_rec",                                                                           \
        "let rec map f xs = match xs case [] then [] case [x, ...rest] then [f x, ...map f " \
        "rest]; map (\\x -> x + 1) [1, 2, 3]"                                                \
    )

static int run_example(
    Buffer *output_, const char *name, const char *snapshot_name, const char *source,
    Buffer *comp_buffer, Buffer *output, Parser *parser
) {
    token    t;
    Compiler c;
    program  prog;

    token_begin(&t, source);
    test_printf("%s\n", name);

    ASSERT_ENUM_EQ(parse_program(parser, &t), PARSE_OK, parse_result_name);

    compiler_init(&c);
    compiler_visit_program(&c, parser->node);
    compiler_write_program(&c, comp_buffer);

    program_read(&prog, (const uint8_t *)comp_buffer->data, comp_buffer->len);

    buffer_printf(output, "%s\n\n", source);
    program_debug_print(&prog, output);

    SNAPSHOT(snapshot_name, output->data);

    compiler_free(&c);
    parser_reset(parser);
    buffer_clear(output);
    buffer_clear(comp_buffer);

    return TEST_OK;
}

TEST(compiler_examples) {
    Parser parser;
    Buffer comp_buffer;
    Buffer output;
    int    result;

    parser_init(&parser);
    buffer_init(&comp_buffer);
    buffer_init(&output);

#define RUN(name, source)                                                                          \
    result = run_example(output_, name, "bytecode_" name, source, &comp_buffer, &output, &parser); \
    if (result != TEST_OK) return result;

    EACH_EXAMPLE(RUN)
#undef RUN

    parser_free(&parser);
    buffer_free(&comp_buffer);
    buffer_free(&output);

    return TEST_OK;
}

TEST(compile_int) {
    ASSERT_INT_EQ(compile_int(STRING("0")), 0);
    ASSERT_INT_EQ(compile_int(STRING("1")), 1);
    ASSERT_INT_EQ(compile_int(STRING("42")), 42);
    ASSERT_INT_EQ(compile_int(STRING("9223372036854775807")), 9223372036854775807);

    return TEST_OK;
}

TEST(compile_string) {
    Buffer out;
    buffer_init(&out);

    compile_string(STRING("\"\""), &out);
    ASSERT_STR_EQ(out.data, "");
    buffer_clear(&out);

    compile_string(STRING("\"foo\""), &out);
    ASSERT_STR_EQ(out.data, "foo");

    compile_string(STRING("\" bar\""), &out);
    ASSERT_STR_EQ(out.data, "foo bar");
    buffer_clear(&out);

    compile_string(STRING("\"\\\"\\n\""), &out);
    ASSERT_STR_EQ(out.data, "\"\n");
    buffer_clear(&out);

    buffer_free(&out);

    return TEST_OK;
}

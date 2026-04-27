#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/parser.h"
#include "lib/snapshot.h"
#include "lib/test.h"

#define EXAMPLES_FILE_PATH "test/compiler_examples.txt"

TEST(compiler_examples) {
    token       t;
    Parser      p;
    Compiler    c;
    FILE       *examples_file;
    Buffer      examples_buffer;
    const char *line;
    size_t      line_len;
    char        line_buffer[0x400];
    Buffer      comp_buffer;
    Buffer      output;
    program     prog;
    int         i;

    examples_file = fopen(EXAMPLES_FILE_PATH, "rb");
    if (!examples_file) {
        perror("Could not open `" EXAMPLES_FILE_PATH "`: ");
        return TEST_FAIL;
    }

    buffer_init(&examples_buffer);
    buffer_read_file(&examples_buffer, examples_file);

    fclose(examples_file);

    parser_init(&p);
    buffer_init(&comp_buffer);
    buffer_init(&output);

    for (i = 1, line = examples_buffer.data, line_len = 0; *line; i++) {
        next_line(&line, &line_len);
        if (line_len == 0) continue;

        strncpy(line_buffer, line, line_len)[line_len] = '\0';
        token_begin(&t, line_buffer);
        test_printf("%s\n", line_buffer);
        ASSERT_ENUM_EQ(parse_program(&p, &t), PARSE_OK, parse_result_name);

        compiler_init(&c);
        compiler_visit_program(&c, p.node);
        compiler_write_program(&c, &comp_buffer);

        program_read(&prog, (const uint8_t *)comp_buffer.data, comp_buffer.len);
        bytecode_debug_print(prog.bytecode, prog.bytecode_len, &output);

        sprintf(line_buffer, "bytecode_%03d", i);
        SNAPSHOT(line_buffer, output.data);

        compiler_free(&c);
        parser_reset(&p);
        buffer_clear(&comp_buffer);
        buffer_clear(&output);
    }

    parser_free(&p);
    buffer_free(&examples_buffer);
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

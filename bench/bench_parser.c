#include "../lib/parser.h"
#include "../vendor/ubench.h"

#define OPERATOR_SET "*+-&|<>$#"

struct parse_binary {
    Parser parser;
    Buffer input;
};

UBENCH_F_SETUP(parse_binary) {
    Parser *p = &ubench_fixture->parser;
    Buffer *buffer = &ubench_fixture->input;

    parser_init(p);
    parser_define_operator(p, "*", 1, 600, ASSOC_LEFT);
    parser_define_operator(p, "+", 1, 500, ASSOC_LEFT);
    parser_define_operator(p, "-", 1, 500, ASSOC_LEFT);
    parser_define_operator(p, "&", 1, 400, ASSOC_LEFT);
    parser_define_operator(p, "|", 1, 400, ASSOC_LEFT);
    parser_define_operator(p, "<", 1, 300, ASSOC_LEFT);
    parser_define_operator(p, ">", 1, 300, ASSOC_LEFT);
    parser_define_operator(p, "$", 1, 200, ASSOC_RIGHT);
    parser_define_operator(p, "#", 1, 100, ASSOC_RIGHT);

    buffer_init(buffer);
    buffer_putc(buffer, 'a');
    srand(0);

    for (int i = 0; i < 10000; i++) {
        char operator = OPERATOR_SET[rand() % (sizeof(OPERATOR_SET) - 1)];
        char operand = 'a' + (rand() % ('z' - 'a'));
        buffer_printf(buffer, " %c %c", operator, operand);
    }
}

UBENCH_F_TEARDOWN(parse_binary) {
    parser_free(&ubench_fixture->parser);
    buffer_free(&ubench_fixture->input);
}

UBENCH_F(parse_binary, parse) {
    token t;
    token_begin(&t, ubench_fixture->input.data);

    parse_result result;
    if ((result = parse_binary(&ubench_fixture->parser, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(result));

    UBENCH_DO_NOTHING(&ubench_fixture->parser);
    parser_reset(&ubench_fixture->parser);
}

#define EXAMPLE_FILE_PATH "examples/html.ham"

struct parse_full_example {
    Parser parser;
    Buffer input;
};

UBENCH_F_SETUP(parse_full_example) {
    parser_init(&ubench_fixture->parser);

    FILE *f = fopen(EXAMPLE_FILE_PATH, "r");
    if (!f) {
        perror("Could not open `" EXAMPLE_FILE_PATH "`: ");
        panic("could not open file");
    }

    buffer_init(&ubench_fixture->input);
    buffer_read_file(&ubench_fixture->input, f);

    fclose(f);
}

UBENCH_F_TEARDOWN(parse_full_example) {
    parser_free(&ubench_fixture->parser);
    buffer_free(&ubench_fixture->input);
}

UBENCH_F(parse_full_example, parse) {
    token t;
    token_begin(&t, ubench_fixture->input.data);

    parse_result result;
    if ((result = parse_program(&ubench_fixture->parser, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(result));

    UBENCH_DO_NOTHING(&ubench_fixture->parser);
    parser_reset(&ubench_fixture->parser);
}

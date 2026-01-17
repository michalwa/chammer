#include "../lib/parser.h"
#include "../vendor/ubench.h"

#define OPERATOR_SET "*+-&|<>$#"

struct parse_binary {
    Parser parser;
    char   buffer[40001];
};

UBENCH_F_SETUP(parse_binary) {
    Parser *p = &ubench_fixture->parser;
    char   *buffer = ubench_fixture->buffer;

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

    strcpy(buffer, "a");
    srand(0);
    char *cursor = buffer + strlen(buffer);

    for (int i = 0; i < 10000; i++) {
        char operator = OPERATOR_SET[rand() % (sizeof(OPERATOR_SET) - 1)];
        char operand = 'a' + (rand() % ('z' - 'a'));
        cursor += sprintf(cursor, " %c %c", operator, operand);
    }
}

UBENCH_F_TEARDOWN(parse_binary) {
    parser_free(&ubench_fixture->parser);
}

UBENCH_F(parse_binary, parse_binary) {
    token t;
    token_begin(&t, ubench_fixture->buffer);

    parse_result result;
    if ((result = parse_binary(&ubench_fixture->parser, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(result));

    UBENCH_DO_NOTHING(&ubench_fixture->parser);
    parser_reset(&ubench_fixture->parser);
}

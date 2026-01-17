#include "../lib/graphviz.h"

int main(void) {
    Buffer input;
    buffer_init(&input);

    char   buffer[0x100];
    size_t read;
    while (read = fread(buffer, 1, 0x100, stdin)) buffer_printf(&input, "%.*s", (int)read, buffer);

    token t;
    token_begin(&t, input.data);

    Parser parser;
    parser_init(&parser);

    parse_result result;
    if ((result = parse_program(&parser, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(result));

    Buffer output;
    buffer_init(&output);
    node_print_dot(parser.node, &output);

    printf("%s", output.data);

    buffer_free(&output);
    parser_free(&parser);
    buffer_free(&input);

    return 0;
}

#include "../lib/lexer.h"

#include <stdio.h>
#include <stdlib.h>

#include "../lib/buffer.h"
#include "example.tokens.h"

#define EXAMPLE_FILE_PATH "test/example.ham"

int test_lexer_example(Buffer *output) {
    FILE *f = fopen(EXAMPLE_FILE_PATH, "r");
    if (!f) {
        fprintf(stderr, "Could not open `" EXAMPLE_FILE_PATH "': ");
        perror(0);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    fread(buffer, size, 1, f);
    fclose(f);
    buffer[size] = 0;

    token actual;
    token_begin(&actual, buffer);

#define _(type, str) { type, str, sizeof(str) - 1 },
    static const token example_tokens[] = { EXAMPLE_TOKENS };
#undef _

    for (size_t i = 0; i < sizeof(example_tokens) / sizeof(token); i++) {
        token expected = example_tokens[i];

        lex_result result = token_next(&actual);

        switch (result) {
        case LEX_OK:
            break;
        case LEX_NOT_FOUND:
            buffer_printf(output, "No more tokens\n");
            return 1;
        case LEX_EOI:
            buffer_printf(output, "Unexpected end of input\n");
            return 1;
        case LEX_NUM:
            buffer_printf(output, "Malformed number\n");
            return 1;
        }

        if (!token_eq(expected, actual)) {
            loc l = token_loc(actual, buffer);
            buffer_printf(
                output, "Expected " F_TOKEN ", got " F_TOKEN " at %u:%u\n", FA_TOKEN(expected),
                FA_TOKEN(actual), l.line + 1, l.col + 1
            );
            return 1;
        }
    }

    return 0;
}

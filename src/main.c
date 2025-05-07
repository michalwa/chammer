#include <stdio.h>
#include <stdlib.h>

#include "example.tokens.h"
#include "example.tree.h"
#include "lexer.h"

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Could not open `%s': ", argv[1]);
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
            fprintf(stderr, "No more tokens\n");
            return 1;
        case LEX_EOI:
            fprintf(stderr, "Unexpected end of input\n");
            return 1;
        case LEX_NUM:
            fprintf(stderr, "Malformed number\n");
            return 1;
        }

        if (!token_eq(expected, actual)) {
            loc l = token_loc(actual, buffer);
            fprintf(
                stderr, "Expected " F_TOKEN ", got " F_TOKEN " at %u:%u\n", FA_TOKEN(expected),
                FA_TOKEN(actual), l.line + 1, l.col + 1
            );
            return 1;
        }

        printf(F_TOKEN "\n", FA_TOKEN(actual));
    }

    printf("----------------\n");

    int indent = 0;

#define _BEGIN(type, ...)                                                              \
    printf("%*s" F_NODE "\n", indent, "", FA_NODE(((node){ type, 0, { 0 }, { 0 } }))); \
    indent += 2;
#define _END indent -= 2;
    EXAMPLE_TREE
#undef _BEGIN
#undef _END

    free(buffer);
}

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "example.tokens.h"

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

    for (size_t i = 0; i < sizeof(EXAMPLE_TOKENS) / sizeof(token); i++) {
        token expected = EXAMPLE_TOKENS[i];

        if (!token_next(&actual)) {
            fprintf(stderr, "Unexpected end of tokens!\n");
            return 1;
        }

        if (!token_eq(expected, actual)) {
            loc l = token_loc(actual, buffer);
            fprintf(
                stderr,
                "Expected %s `%.*s', got %s `%.*s' at %u:%u\n",
                token_name(expected), (int)expected.len, expected.str,
                token_name(actual), (int)actual.len, actual.str,
                l.line + 1, l.col + 1
            );
            return 1;
        }

        printf("%-8s `%.*s'\n", token_name(actual), (int)actual.len, actual.str);
    }

    free(buffer);
}

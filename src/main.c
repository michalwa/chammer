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

    token actual = {0};

    for (size_t i = 0; i < sizeof(EXAMPLE_TOKENS) / sizeof(token); i++) {
        token expected = EXAMPLE_TOKENS[i];

        if (!next_token(buffer, &actual)) {
            fprintf(stderr, "Unexpected end of tokens!\n");
            return 1;
        }

        if (!token_eq(expected, actual)) {
            fprintf(
                stderr,
                "Expected %2d `%.*s', got %2d `%.*s'\n",
                expected.type, expected.len, expected.str,
                actual.type, actual.len, actual.str
            );
            return 1;
        }
    }

    free(buffer);
    printf("OK\n");
}

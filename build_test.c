#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_NAME_PREFIX "int test_"
#define TEST_NAME_OFFSET (sizeof("int ") - 1)

int main(int argc, const char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output_file> <test_file...>\n", argv[0]);
        return 1;
    }

    FILE *output_file = fopen(argv[1], "w");

    for (int i = 2; i < argc; i++) {
        FILE *input_file = fopen(argv[i], "r");
        fseek(input_file, 0, SEEK_END);
        size_t size = ftell(input_file);
        rewind(input_file);

        char *buffer = malloc(size + 1);
        fread(buffer, size, 1, input_file);
        fclose(input_file);
        buffer[size] = 0;

        const char *c = buffer;
        while (c = strstr(c, TEST_NAME_PREFIX)) {
            c += TEST_NAME_OFFSET;

            int len = 0;
            while (isalnum(c[len]) || c[len] == '_') len++;

            fprintf(output_file, "TEST(%.*s);\n", len, c);
        }

        free(buffer);
    }

    fclose(output_file);
}

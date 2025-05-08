#include <stdio.h>
#include <unistd.h>

#include "../lib/buffer.h"

#define RED(str)   "\033[0;31m" str "\033[0m"
#define GREEN(str) "\033[0;32m" str "\033[0m"

#define TEST(name)                          \
    int name(Buffer *);                     \
    run_test(&name, #name, _RUN_TEST_ARGS);

typedef struct {
    int passed;
    int failed;
} stats;

void run_test(int (*test)(Buffer *), const char *label, stats *stats, Buffer *output) {
    Buffer buffer;
    buffer_init(&buffer);

    printf("%-30s", label);

    if (test(&buffer)) {
        stats->failed++;
        printf(RED("failed") "\n");
        buffer_printf(output, "\n-------- %s --------\n" F_BUFFER "\n", label, FA_BUFFER(buffer));
    } else {
        stats->passed++;
        printf(GREEN("ok") "\n");
    }

    buffer_free(&buffer);
}

int main(void) {
    stats  stats = { 0 };
    Buffer output;
    buffer_init(&output);

#define _RUN_TEST_ARGS &stats, &output
    TEST(test_lexer_example);
#undef _RUN_TEST_ARGS

    printf("\n%d passed, %d failed\n", stats.passed, stats.failed);

    if (stats.failed) printf(F_BUFFER, FA_BUFFER(output));

    buffer_free(&output);

    return !!stats.failed;
}

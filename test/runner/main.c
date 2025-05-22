#include <stdio.h>
#include <unistd.h>

#include "../../lib/buffer.h"
#include "../lib/test.h"
#include "tests.gen.h"

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
        buffer_printf(output, "-------- %s --------\n" F_BUFFER "\n", label, FA_BUFFER(buffer));
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

#define _(name)                                     \
    int test_##name(Buffer *);                      \
    run_test(&test_##name, #name, &stats, &output);
    TESTS
#undef _

    printf("\n%d passed, %d failed\n", stats.passed, stats.failed);

    if (stats.failed) printf("\n" F_BUFFER, FA_BUFFER(output));

    buffer_free(&output);

    return !!stats.failed;
}

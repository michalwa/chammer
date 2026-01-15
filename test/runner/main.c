#include <stdio.h>

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

    printf("%-40s", label);

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

int main(int argc, const char **argv) {
    stats  stats = { 0 };
    Buffer output;
    buffer_init(&output);

    const char *test_name = NULL;

    if (argc == 2) {
        test_name = argv[1];
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s [<test_name>]\n", argv[0]);
        return 1;
    }

#define RUN(name)              \
    int test_##name(Buffer *); \
    if (!test_name || strcmp(test_name, #name) == 0) run_test(&test_##name, #name, &stats, &output);
    EACH_TEST(RUN)
#undef RUN

    printf("\n%d passed, %d failed\n", stats.passed, stats.failed);

    if (stats.failed) printf("\n" F_BUFFER, FA_BUFFER(output));

    buffer_free(&output);

    return !!stats.failed;
}

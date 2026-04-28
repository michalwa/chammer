#ifndef HAMMER_SNAPSHOT_H_
#define HAMMER_SNAPSHOT_H_

#include "../../lib/buffer.h"

#define SNAPSHOT(name, data)                        \
    do {                                            \
        int result = snapshot(output_, name, data); \
        if (result != TEST_OK) return result;       \
    } while (0)

/*
 * Writes a naive human-readable line-based diff of the string `b` relative to `a`
 */
void snapshot_diff(Buffer *output, const char *a, const char *b);
/*
 * For usage in tests, see the `SNAPSHOT` macro.
 *
 * In normal mode, asserts that `data` is the same as saved in the snapshot. In
 * review mode (`HAMMER_SNAPSHOT_REVIEW=1`) prompts the user whether to update
 * the snapshot if it differs. Writes all user-facing output to `output`.
 */
int  snapshot(Buffer *output, const char *name, const char *data);

#endif // HAMMER_SNAPSHOT_H_

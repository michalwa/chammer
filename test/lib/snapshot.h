#ifndef SNAPSHOT_H_
#define SNAPSHOT_H_

#include <stdio.h>

#include "../../lib/buffer.h"

#define SNAPSHOT(name, file, body)                                 \
    {                                                              \
        FILE *file = tmpfile();                                    \
        if (!file) return TEST_FAIL;                               \
        body;                                                      \
        int result = snapshot(output_, SNAPSHOT_FILE(name), file); \
        fclose(file);                                              \
        if (result != TEST_OK) return result;                      \
    }

#define SNAPSHOT_FILE(name) "test/snapshots/" name ".txt"

int snapshot(Buffer *output, const char *filename, FILE *current);

#endif // SNAPSHOT_H_

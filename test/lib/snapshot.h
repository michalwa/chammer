#ifndef SNAPSHOT_H_
#define SNAPSHOT_H_

#include <stdio.h>

#include "../../lib/buffer.h"

#define SNAPSHOT(name, data)                        \
    {                                               \
        int result = snapshot(output_, name, data); \
        if (result != TEST_OK) return result;       \
    }

int snapshot(Buffer *output, const char *name, const char *data);

#endif // SNAPSHOT_H_

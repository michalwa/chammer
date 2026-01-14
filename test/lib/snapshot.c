#include "snapshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

static void next_line(const char **line, size_t *len) {
    *line += *len;
    if (**line == '\n') (*line)++;
    *len = strcspn(*line, "\n");
}

static void snapshot_diff(Buffer *output, const char *a, const char *b) {
    const char *a_line = a, *b_line = b;
    size_t      a_len = 0, b_len = 0;

    next_line(&a_line, &a_len);
    next_line(&b_line, &b_len);

    while (*a_line || *b_line) {
        if (*a_line && *b_line) {
            if (a_len == b_len && strncmp(a_line, b_line, a_len) == 0) {
                buffer_printf(output, "  %.*s\n", (int)a_len, a_line);
            } else {
                buffer_printf(output, RED("- %.*s\n"), (int)a_len, a_line);
                buffer_printf(output, GREEN("+ %.*s\n"), (int)b_len, b_line);
            }

            next_line(&a_line, &a_len);
            next_line(&b_line, &b_len);
        } else if (*a_line) {
            buffer_printf(output, RED("- %.*s\n"), (int)a_len, a_line);
            next_line(&a_line, &a_len);
        } else if (*b_line) {
            buffer_printf(output, GREEN("+ %.*s\n"), (int)b_len, b_line);
            next_line(&b_line, &b_len);
        }
    }
}

static void snapshot_save(const char *filename, const char *data) {
    FILE *new = fopen(filename, "w");
    fprintf(new, "%s", data);
    fclose(new);
}

int snapshot(Buffer *output, const char *name, const char *data) {
    int status = TEST_OK;

    Buffer filename;
    buffer_init(&filename);
    buffer_printf(&filename, "test/snapshots/%s.txt", name);

    FILE *old = fopen(filename.data, "r");

    if (old) {
        Buffer old_buf;
        buffer_init(&old_buf);
        fread_to_buffer(old, &old_buf);

        if (strcmp(old_buf.data, data) != 0) {
            if (getenv("HAMMER_SNAPSHOT_REVIEW")) {
                Buffer tmp;
                buffer_init(&tmp);

                snapshot_diff(&tmp, data, old_buf.data);
                printf(
                    "\n\nSnapshot changed: %s\n\n" F_BUFFER "\nAccept new version? (y/N) ",
                    filename.data, FA_BUFFER(tmp)
                );

                buffer_free(&tmp);

                if (getchar() == 'y')
                    snapshot_save(filename.data, data);
                else
                    status = TEST_FAIL;
            } else {
                buffer_printf(output, "Snapshot changed: %s\n\n", filename.data);
                snapshot_diff(output, data, old_buf.data);
                status = TEST_FAIL;
            }
        }

        buffer_free(&old_buf);
        fclose(old);
    } else {
        snapshot_save(filename.data, data);
    }

    buffer_free(&filename);

    return status;
}

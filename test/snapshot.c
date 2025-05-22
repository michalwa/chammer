#include "snapshot.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

static void next_line(string *line) {
    line->data += line->len;
    while (*line->data == '\n') line->data++;
    line->len = strcspn(line->data, "\n");
}

static void snapshot_diff(Buffer *output, const Buffer *a, const Buffer *b) {
    const char *a_end = a->data + a->len;
    const char *b_end = b->data + b->len;
    string      a_line = { a->data, 0 };
    string      b_line = { b->data, 0 };

    next_line(&a_line);
    next_line(&b_line);

    while (a_line.data < a_end || b_line.data < b_end) {
        if (a_line.data < a_end && b_line.data < b_end) {
            if (string_eq(a_line, b_line)) {
                buffer_printf(output, "  " F_STRING "\n", FA_STRING(a_line));
            } else {
                buffer_printf(output, RED("- " F_STRING "\n"), FA_STRING(a_line));
                buffer_printf(output, GREEN("+ " F_STRING "\n"), FA_STRING(b_line));
            }

            next_line(&a_line);
            next_line(&b_line);
        } else if (a_line.data < a_end) {
            buffer_printf(output, RED("- " F_STRING "\n"), FA_STRING(a_line));
            next_line(&a_line);
        } else if (b_line.data < b_end) {
            buffer_printf(output, GREEN("+ " F_STRING "\n"), FA_STRING(b_line));
            next_line(&b_line);
        }
    }
}

static void snapshot_save(const char *filename, const Buffer *new_buf) {
    FILE *new = fopen(filename, "w");
    fprintf(new, F_BUFFER, FA_BUFFER(*new_buf));
    fclose(new);
}

int snapshot(Buffer *output, const char *filename, FILE *new) {
    int status = TEST_OK;

    Buffer new_buf;
    buffer_init(&new_buf);
    fread_to_buffer(new, &new_buf);

    FILE *old = fopen(filename, "r");

    if (old) {
        Buffer old_buf;
        buffer_init(&old_buf);
        fread_to_buffer(old, &old_buf);

        if (!string_eq(buffer_to_string(old_buf), buffer_to_string(new_buf))) {
            if (getenv("HAMMER_SNAPSHOT_REVIEW")) {
                Buffer tmp;
                buffer_init(&tmp);

                snapshot_diff(&tmp, &new_buf, &old_buf);
                printf(
                    "\n\nSnapshot changed: %s\n\n" F_BUFFER "\nAccept new version? (y/N) ",
                    filename, FA_BUFFER(tmp)
                );

                buffer_free(&tmp);

                if (getchar() == 'y')
                    snapshot_save(filename, &new_buf);
                else
                    status = TEST_FAIL;
            } else {
                buffer_printf(output, "Snapshot changed: %s\n\n", filename);
                snapshot_diff(output, &new_buf, &old_buf);
                status = TEST_FAIL;
            }
        }

        buffer_free(&old_buf);
        fclose(old);
    } else {
        snapshot_save(filename, &new_buf);
    }

    buffer_free(&new_buf);
    return status;
}

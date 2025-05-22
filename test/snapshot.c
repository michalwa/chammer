#include "snapshot.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"

static void next_line(const char *c, size_t *start, size_t *len) {
    *start += *len;
    while (c[*start] == '\n') (*start)++;
    *len = strcspn(c + *start, "\n");
}

static void snapshot_diff(Buffer *output, const Buffer *a, const Buffer *b) {
    size_t a_start = 0, b_start = 0, a_len = 0, b_len = 0;

    next_line(a->data, &a_start, &a_len);
    next_line(b->data, &b_start, &b_len);

    while (a_start < a->len || b_start < b->len) {
        if (a_start < a->len && b_start < b->len) {
            if (a_len == b_len && strncmp(a->data + a_start, b->data + b_start, a_len) == 0) {
                buffer_printf(output, "  %.*s\n", (int)a_len, a->data + a_start);
            } else {
                buffer_printf(output, RED("- %.*s\n"), (int)a_len, a->data + a_start);
                buffer_printf(output, GREEN("+ %.*s\n"), (int)b_len, b->data + b_start);
            }

            next_line(a->data, &a_start, &a_len);
            next_line(b->data, &b_start, &b_len);
        } else if (a_start < a->len) {
            buffer_printf(output, RED("- %.*s\n"), (int)a_len, a->data + a_start);
            next_line(a->data, &a_start, &a_len);
        } else if (b_start < b->len) {
            buffer_printf(output, GREEN("+ %.*s\n"), (int)b_len, b->data + b_start);
            next_line(a->data, &a_start, &a_len);
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

        if (new_buf.len != old_buf.len || strncmp(new_buf.data, old_buf.data, new_buf.len) != 0) {
            if (getenv("HAMMER_SNAPSHOT_REVIEW")) {
                Buffer tmp;
                buffer_init(&tmp);

                snapshot_diff(&tmp, &new_buf, &old_buf);
                printf(
                    "\n\nSnapshot changed: %s\n\n" F_BUFFER "\nAccept new version? (y/N) ",
                    filename, FA_BUFFER(tmp)
                );

                buffer_free(&tmp);

                if (getchar() == 'y') {
                    snapshot_save(filename, &new_buf);
                    printf("Saved\n");
                } else {
                    status = TEST_FAIL;
                }
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

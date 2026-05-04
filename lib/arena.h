#ifndef HAMMER_ARENA_H_
#define HAMMER_ARENA_H_

#include <stdbool.h>
#include <stddef.h>

#define arena_init(arena, item_type) arena_init_(arena, sizeof(item_type))
#define arena_init_block_size(arena, item_type, block_size)      \
    arena_init_block_size_(arena, sizeof(item_type), block_size)

typedef struct arena_block arena_block;

/**
 * A linked-list arena allocator
 *
 * Homogenous growable container with contents pinned in memory so as not to
 * invalidate pointers.
 */
typedef struct {
    size_t       item_size;
    size_t       items_per_block;
    size_t       len;
    arena_block *head;
} Arena;

typedef struct {
    Arena       *arena;
    arena_block *block;
    size_t       index;
    void        *item;
} arena_iter;

void       arena_init_(Arena *, size_t item_size);
void       arena_init_block_size_(Arena *, size_t item_size, size_t items_per_block);
void       arena_free(Arena *);
void      *arena_push(Arena *);
void      *arena_push_zeroed(Arena *);
void      *arena_head(Arena *);
void       arena_truncate(Arena *, size_t len);
void       arena_clear(Arena *);
size_t     arena_count_blocks(Arena *);
arena_iter arena_iter_begin(Arena *);
bool       arena_iter_next(arena_iter *);

#endif // HAMMER_ARENA_H_

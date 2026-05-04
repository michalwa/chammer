#include "arena.h"

#include <inttypes.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define ARENA_BLOCK_MAX_DEFAULT_SIZE 0x400

struct arena_block {
    arena_block *next;
    char         data[];
};

static arena_block *arena_block_new(size_t size) {
    arena_block *block = malloc(sizeof(arena_block) + size);
    block->next = NULL;
    return block;
}

void arena_init_(Arena *a, size_t item_size) {
    arena_init_block_size_(a, item_size, ARENA_BLOCK_MAX_DEFAULT_SIZE / item_size);
}

void arena_init_block_size_(Arena *a, size_t item_size, size_t items_per_block) {
    a->item_size = item_size;
    a->items_per_block = items_per_block;
    a->len = 0;
    a->head = arena_block_new(a->item_size * a->items_per_block);
}

void arena_free(Arena *a) {
    arena_block *block = a->head;

    while (block) {
        arena_block *next = block->next;
        free(block);
        block = next;
    }
}

void *arena_push(Arena *a) {
    size_t       offset = a->len;
    arena_block *block = a->head;

    while (offset >= a->items_per_block) {
        if (!block->next) block->next = arena_block_new(a->item_size * a->items_per_block);

        block = block->next;
        offset -= a->items_per_block;
    }

    a->len++;
    return block->data + offset * a->item_size;
}

void *arena_push_zeroed(Arena *a) {
    void *ptr = arena_push(a);
    memset(ptr, 0, a->item_size);
    return ptr;
}

inline void *arena_head(Arena *a) {
    return a->head->data;
}

void arena_truncate(Arena *a, size_t len) {
    debug_assert(len <= a->len);
    a->len = len;
}

void arena_clear(Arena *a) {
    a->len = 0;
}

size_t arena_count_blocks(Arena *a) {
    size_t n = 0;
    for (arena_block *block = a->head; block; block = block->next) n++;
    return n;
}

arena_iter arena_iter_begin(Arena *a) {
    return (arena_iter){
        .arena = a,
        .block = a->head,
        .index = 0,
        .item = NULL,
    };
}

bool arena_iter_next(arena_iter *i) {
    if (i->item && ++i->index >= i->arena->len) return false;

    if (i->index > 0 && i->index % i->arena->items_per_block == 0) i->block = i->block->next;

    i->item = i->block->data + (i->index % i->arena->items_per_block) * i->arena->item_size;
    return true;
}

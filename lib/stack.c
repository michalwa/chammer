#include "stack.h"

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define STACK_BLOCK_MAX_DEFAULT_SIZE 0x400

struct stack_block {
    stack_block *next;
    char         data[];
};

static stack_block *stack_block_new(size_t size) {
    stack_block *block = malloc(sizeof(stack_block) + size);
    block->next = NULL;
    return block;
}

void stack_init_(Stack *s, size_t item_size) {
    stack_init_block_size_(s, item_size, STACK_BLOCK_MAX_DEFAULT_SIZE / item_size);
}

void stack_init_block_size_(Stack *s, size_t item_size, size_t items_per_block) {
    s->item_size = item_size;
    s->items_per_block = items_per_block;
    s->size = 0;
    s->head = stack_block_new(s->item_size * s->items_per_block);
}

void stack_free(Stack *s) {
    stack_block *block = s->head;

    while (block) {
        stack_block *next = block->next;
        free(block);
        block = next;
    }
}

void *stack_push(Stack *s) {
    size_t       offset = s->size;
    stack_block *block = s->head;

    while (offset >= s->items_per_block) {
        if (!block->next)
            block->next = stack_block_new(s->item_size * s->items_per_block);

        block = block->next;
        offset -= s->items_per_block;
    }

    s->size++;
    return block->data + offset * s->item_size;
}

void *stack_push_zeroed(Stack *s) {
    void *ptr = stack_push(s);
    memset(ptr, 0, s->item_size);
    return ptr;
}

void stack_pop(Stack *s) {
    debug_assert(s->size > 0);
    s->size--;
}

void *stack_get(Stack *s, size_t index) {
    debug_assert(index < s->size);

    size_t       offset = index;
    stack_block *block = s->head;

    while (offset >= s->items_per_block) {
        block = block->next;
        offset -= s->items_per_block;
    }

    return block->data + offset * s->item_size;
}

inline void *stack_top(Stack *s) {
    return stack_get(s, s->size - 1);
}

void stack_truncate(Stack *s, size_t size) {
    debug_assert(size <= s->size);
    s->size = size;
}

void stack_clear(Stack *s) {
    s->size = 0;
}

stack_iter stack_iter_begin(Stack *s) {
    return (stack_iter){
        .stack = s,
        .block = s->head,
        .index = 0,
        .item = NULL,
    };
}

bool stack_iter_next(stack_iter *i) {
    if (i->index >= i->stack->size) return false;
    if (i->item && ++i->index >= i->stack->size) return false;

    if (i->index > 0 && i->index % i->stack->items_per_block == 0)
        i->block = i->block->next;

    i->item = i->block->data + (i->index % i->stack->items_per_block) * i->stack->item_size;
    return true;
}

size_t stack_count_blocks(Stack *s) {
    size_t n = 0;
    for (stack_block *block = s->head; block; block = block->next)
        n++;
    return n;
}

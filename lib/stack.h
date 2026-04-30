#ifndef HAMMER_STACK_H_
#define HAMMER_STACK_H_

#include <stdbool.h>
#include <stddef.h>

#define stack_init(stack, item_type) stack_init_(stack, sizeof(item_type))
#define stack_init_block_size(stack, item_type, block_size)      \
    stack_init_block_size_(stack, sizeof(item_type), block_size)

typedef struct stack_block stack_block;

/**
 * Like a Vector, but pinned in memory so as not to invalidate pointers.
 * Less efficient than a Vector if pinning is not needed.
 */
typedef struct {
    size_t       item_size;
    size_t       items_per_block;
    // TODO: Rename to `len` for consistency
    size_t       size;
    stack_block *head;
} Stack;

typedef struct {
    Stack       *stack;
    stack_block *block;
    size_t       index;
    void        *item;
} stack_iter;

void       stack_init_(Stack *, size_t item_size);
void       stack_init_block_size_(Stack *, size_t item_size, size_t items_per_block);
void       stack_free(Stack *);
void      *stack_push(Stack *);
void      *stack_push_zeroed(Stack *);
bool       stack_pop(Stack *, void *);
void      *stack_get(Stack *, size_t index);
void      *stack_top(Stack *);
void       stack_truncate(Stack *, size_t size);
void       stack_clear(Stack *);
size_t     stack_count_blocks(Stack *);
stack_iter stack_iter_begin(Stack *);
bool       stack_iter_next(stack_iter *);

#endif // HAMMER_STACK_H_

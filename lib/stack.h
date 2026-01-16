#ifndef STACK_H_
#define STACK_H_

#include <inttypes.h>

#include "string.h"

#define stack_push(stack, item_type) (item_type *)stack_push_(stack, sizeof(item_type))
#define stack_push_zeroed(stack, item_type)                   \
    (item_type *)stack_push_zeroed_(stack, sizeof(item_type))

typedef struct stack_block stack_block;

typedef struct {
    stack_block *head;
    size_t       block_size;
    size_t       cursor;
} Stack;

typedef struct stack_item *stack_ptr;

void      stack_init(Stack *);
void      stack_init_block_size(Stack *, size_t block_size);
void      stack_free(Stack *);
void     *stack_push_(Stack *, size_t size);
void     *stack_push_zeroed_(Stack *, size_t size);
stack_ptr stack_top(Stack *);
void      stack_rewind(Stack *, stack_ptr);
void      stack_clear(Stack *);

#endif // STACK_H_

#include "../lib/stack.h"

#include <stdlib.h>

#include "test.h"

TEST(stack) {
    Stack stack;
    stack_init_block_size(&stack, 8);

    stack_ptr top = stack_top(&stack);

    stack_push_(&stack, 6);
    ASSERT_INT_EQ(stack.cursor, 6);

    stack_push_(&stack, 6);
    ASSERT_INT_EQ(stack.cursor, 8 + 6);

    stack_rewind(&stack, top);
    ASSERT_INT_EQ(stack.cursor, 0);

    stack_push_(&stack, 8);
    ASSERT_INT_EQ(stack.cursor, 8);

    return TEST_OK;
}

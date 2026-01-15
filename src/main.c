#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../lib/parser.h"
#include "example.tree.h"

node *node_begin(Stack *s, node *parent, int argc, ...) {
    va_list args;
    va_start(args, argc);

    node *n = stack_push_zeroed(s, node);

    if (argc-- > 0) n->type = va_arg(args, node_type);
    if (argc-- > 0) n->token.type = va_arg(args, token_type);
    if (argc-- > 0) {
        n->token.str = va_arg(args, char *);
        n->token.len = strlen(n->token.str);
    }
    if (argc-- > 0) n->flags = va_arg(args, node_flags);

    if (parent) node_add_children(parent, n);

    va_end(args);
    return n;
}

int main(void) {
    Stack stack;
    stack_init(&stack);

    node *n = 0;

#define _BEGIN(...) n = node_begin(&stack, n, ARGC(__VA_ARGS__), __VA_ARGS__);
#define _END                      \
    if (n->parent) n = n->parent;
    EXAMPLE_TREE
#undef _BEGIN
#undef _END

    Buffer output;
    buffer_init(&output);

    node_print(*n, &output);

    printf(F_BUFFER "\n", FA_BUFFER(output));
    buffer_free(&output);

    stack_free(&stack);
}

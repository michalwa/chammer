#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../lib/parser.h"
#include "example.tree.h"

#define ARGC(...)                                         ARGC_(__VA_ARGS__, ARGC_SEQ_)
#define ARGC_(...)                                        ARGC_NTH_(__VA_ARGS__)
#define ARGC_NTH_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define ARGC_SEQ_                                         8, 7, 6, 5, 4, 3, 2, 1, 0

void node_begin(int indent, int argc, ...) {
    va_list args;
    va_start(args, argc);

    node node = { 0 };

    if (argc-- > 0) node.type = va_arg(args, node_type);
    if (argc-- > 0) node.token.type = va_arg(args, token_type);
    if (argc-- > 0) {
        node.token.str = va_arg(args, char *);
        node.token.len = strlen(node.token.str);
    }
    if (argc-- > 0) node.flags = va_arg(args, node_flags);

    printf("%*s", indent, "");
    node_print(node, stdout);
    printf("\n");

    va_end(args);
}

int main(void) {
    int indent = 0;

#define _BEGIN(...)                                     \
    node_begin(indent, ARGC(__VA_ARGS__), __VA_ARGS__); \
    indent += 2;
#define _END indent -= 2;
    EXAMPLE_TREE
#undef _BEGIN
#undef _END
}

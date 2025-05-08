#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "../test/example.tree.h"
#include "parser.h"

#define ARGC(...)                                         ARGC_(__VA_ARGS__, ARGC_SEQ_)
#define ARGC_(...)                                        ARGC_NTH_(__VA_ARGS__)
#define ARGC_NTH_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define ARGC_SEQ_                                         8, 7, 6, 5, 4, 3, 2, 1, 0

void node_begin(int argc, node_type type, ...) {
    va_list args;
    va_start(args, type);

    node node;
    node.type = type;

    if (argc-- > 0) node.token.type = va_arg(args, token_type);
    if (argc-- > 0) node.token.str = va_arg(args, char *);
    if (argc-- > 0) node.flags = va_arg(args, node_flags);
    if (argc-- > 0) {
        char *str_parsed = va_arg(args, char *);
        node.str_parsed = (string){ str_parsed, strlen(str_parsed) };
    }

    printf(F_NODE "\n", FA_NODE(node));

    va_end(args);
}

void node_end(void) {}

int main(void) {
#define _BEGIN(...) node_begin(ARGC(__VA_ARGS__), __VA_ARGS__);
#define _END        node_end();
    EXAMPLE_TREE
#undef _BEGIN
#undef _END
}

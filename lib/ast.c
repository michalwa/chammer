#include "ast.h"

#include <stdarg.h>

#include "utils.h"

const char *node_type_name(node_type value) {
    RETURN_ENUM_NAME(node_type, value, EACH_NODE_TYPE);
}

bool node_has_token(node n) {
    switch (n.type) {
    case N_IDENT:
    case N_STRING:
    case N_INT:
    case N_DEC:
    case N_PIDENT:
    case N_UNARY:
    case N_BINARY:
    case N_PAPPLY:
    case N_PALIAS: return true;
    case N_PLTAIL: return n.flags & NF_NAMED;
    default: return false;
    }
}

static void node_print_flags(node n, Buffer *b) {
    const char *prefix = "";

#define CHECK(name, node_type, value)            \
    if (n.type == node_type && n.flags & name) { \
        buffer_printf(b, "%s" #name, prefix);    \
        prefix = " ";                            \
    }

    EACH_NODE_FLAG(CHECK)
#undef CHECK
}

void node_print_(node n, Buffer *b, int indent) {
    buffer_printf(b, "%*s%s ", indent, "", node_type_name(n.type));
    node_print_flags(n, b);
    if (node_has_token(n)) buffer_printf(b, " (" F_TOKEN ")", FA_TOKEN(n.token));

    node *child = n.first_child;
    while (child) {
        buffer_putc(b, '\n');
        node_print_(*child, b, indent + 2);
        child = child->next_sibling;
    }
}

inline void node_print(node n, Buffer *b) {
    node_print_(n, b, 0);
}

void node_add_children_(node *parent, int n, ...) {
    va_list args;
    va_start(args, n);

    node **last_child = &parent->first_child;
    while (*last_child) last_child = &(*last_child)->next_sibling;

    while (n-- > 0) {
        node *child = va_arg(args, node *);
        *last_child = child;
        last_child = &(*last_child)->next_sibling;
    }

    va_end(args);
}

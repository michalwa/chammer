#include "graphviz.h"

#include "parser.h"

static void node_print_dot_(node *n, Buffer *b) {
    buffer_printf(b, "  node_%p [label=\"%s", n, node_type_name(n->type));

    if (n->type != N_STRING && node_has_token(*n))
        buffer_printf(b, " (%.*s)", (int)n->token.len, n->token.str);

    buffer_printf(b, "\"];\n", n, node_type_name(n->type));

    for (node *child = n->first_child; child; child = child->next_sibling) {
        node_print_dot_(child, b);
        buffer_printf(b, "  node_%p -> node_%p;\n", n, child);
    }
}

void node_print_dot(node *n, Buffer *b) {
    buffer_printf(b, "digraph {\n  node [shape=box,fontname=monospace];\n");
    node_print_dot_(n, b);
    buffer_printf(b, "}\n");
}

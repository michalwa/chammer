#ifndef HAMMER_GRAPHVIZ_H_
#define HAMMER_GRAPHVIZ_H_

#include "ast.h"

/*
 * Prints a representation of the AST as a DOT language graph description
 */
void node_print_dot(node *, Buffer *);

#endif // HAMMER_GRAPHVIZ_H_

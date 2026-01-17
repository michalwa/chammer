#ifndef GRAPHVIZ_H_
#define GRAPHVIZ_H_

#include "parser.h"

/*
 * Prints a representation of the AST as a DOT language graph description
 */
void node_print_dot(node *, Buffer *);

#endif // GRAPHVIZ_H_

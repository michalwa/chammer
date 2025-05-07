#include "parser.h"

#define _(name) [name] = #name,
static const char *NODE_NAMES[] = { NODE_TYPES };
#undef _

const char *node_name(node n) {
    return NODE_NAMES[n.type];
}

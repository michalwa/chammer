#include "parser.h"

#define _(name) [name] = #name,
static const char *NODE_NAMES[] = { NODE_TYPES };
#undef _

static bool node_has_token(node n) {
    switch (n.type) {
    case N_IDENT:
    case N_STRING:
    case N_INT:
    case N_DEC:
    case N_PIDENT:
    case N_UNARY:
    case N_BINARY:
    case N_PAPPLY:
    case N_PALIAS:
        return true;
    case N_PLTAIL:
        return n.flags & NF_NAMED;
    default:
        return false;
    }
}

static void node_print_flags(node_flags flags, FILE *f) {
    const char *prefix = "";

#define _(name, _)                      \
    if (flags & name) {                 \
        fprintf(f, "%s" #name, prefix); \
        prefix = " ";                   \
    }

    NODE_FLAGS
#undef _
}

const char *node_name(node n) {
    return NODE_NAMES[n.type];
}

void node_print(node n, FILE *f) {
    fprintf(f, "%s ", NODE_NAMES[n.type]);
    node_print_flags(n.flags, f);
    if (node_has_token(n)) fprintf(f, " (" F_TOKEN ")", FA_TOKEN(n.token));
    if (n.type == N_STRING) fprintf(f, " = `" F_STRING "'", FA_STRING(n.str_parsed));
}

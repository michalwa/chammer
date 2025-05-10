#include "parser.h"

#define _(name) [N_##name] = "N_" #name,
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
}

void parser_init(Parser *p) {
    stack_init(&p->stack);
}

/* * * Helpers * * */

#define PARSER(name) parse_result parse_##name(Parser *p, token *tokens)

#define PARSER_BEGIN                      \
    stack_ptr st_ = stack_top(&p->stack); \
    token     current_token = *tokens;

#define PARSER_FAIL(result)       \
    stack_rewind(&p->stack, st_); \
    return result;

#define PARSER_END           \
    *tokens = current_token; \
    return PARSE_OK;

#define NEXT_TOKEN                                            \
    p->lex_result = token_next(&current_token);               \
    if (p->lex_result != LEX_OK) { PARSER_FAIL(PARSE_ELEX); }

#define THEN_TOKEN(token_type, var) \
    THEN_TOKEN_(token_type);        \
    token var = current_token;

#define THEN_TOKEN_(token_type)                                               \
    NEXT_TOKEN;                                                               \
    p->expected_token = (token_type);                                         \
    if (current_token.type != p->expected_token) { PARSER_FAIL(PARSE_ETOK); }

#define THEN(parser, var) \
    THEN_(parser);        \
    node *var = p->node;

#define THEN_(parser)                                            \
    {                                                            \
        parse_result result = parse_##parser(p, &current_token); \
        if (result != PARSE_OK) { PARSER_FAIL(result); }         \
    }

#define PARSER_ATOM(name, token_type)                 \
    PARSER(name) {                                    \
        PARSER_BEGIN;                                 \
        THEN_TOKEN_(token_type);                      \
        node *n = stack_push_zeroed(&p->stack, node); \
        n->type = N_##name;                           \
        n->token = current_token;                     \
        PARSER_END;                                   \
    }

/* * * Parsers * * */

#define PARSERS   \
    NODE_TYPES    \
    _(pattern)    \
    _(expr)       \
    _(expr_unary)

#define _(name) PARSER(name);
PARSERS
#undef _

PARSER_ATOM(IDENT, T_IDENT)
PARSER_ATOM(STRING, T_STRING)
PARSER_ATOM(INT, T_INT)
PARSER_ATOM(DEC, T_DEC)

PARSER(ASSIGN) {
    PARSER_BEGIN;

    THEN_TOKEN_(T_LET);
    THEN(pattern, lhs);
    THEN_TOKEN_(T_EQ);
    THEN(expr, rhs);
    THEN_TOKEN_(T_SEMI);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_ASSIGN;
    p->node->first_child = lhs;
    lhs->next_sibling = rhs;

    PARSER_END;
}

PARSER(TUPLE) {
    PARSER_BEGIN;

    THEN_TOKEN_(T_POPEN);
    THEN(expr, first_item);
    node *last_item = first_item;

    while (true) {
        NEXT_TOKEN;

        if (current_token.type == T_COMMA) {
            token      peek = current_token;
            lex_result peek_result = token_next(&peek);
            if (peek_result == LEX_OK && peek.type == T_PCLOSE) break;

            THEN(expr, item);
            last_item->next_sibling = item;
            last_item = item;
        } else if (current_token.type == T_PCLOSE && last_item != first_item) {
            break;
        } else {
            p->expected_token = T_COMMA;
            PARSER_FAIL(PARSE_ETOK);
        }
    }

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_TUPLE;
    p->node->first_child = first_item;

    PARSER_END;
}

PARSER(SPREAD) {
    PARSER_BEGIN;

    THEN_TOKEN_(T_ELLIPS);
    THEN(expr, expr);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_SPREAD;
    p->node->first_child = expr;

    PARSER_END;
}

PARSER(UNARY) {
    PARSER_BEGIN;

    THEN_TOKEN(T_OP, op);
    THEN(expr_unary, expr);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_UNARY;
    p->node->token = op;
    p->node->first_child = expr;

    PARSER_END;
}

PARSER(pattern) {
    PARSER_BEGIN;
    PARSER_END;
}

PARSER(expr) {
    PARSER_BEGIN;
    PARSER_END;
}

PARSER(expr_unary) {
    PARSER_BEGIN;
    PARSER_END;
}

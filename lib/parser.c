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

void node_print_(node n, FILE *f, int indent) {
    fprintf(f, "%*s%s ", indent, "", NODE_NAMES[n.type]);
    node_print_flags(n.flags, f);
    if (node_has_token(n)) fprintf(f, " (" F_TOKEN ")", FA_TOKEN(n.token));

    node *child = n.first_child;
    while (child) {
        putc('\n', f);
        node_print_(*child, f, indent + 2);
        child = child->next_sibling;
    }
}

inline void node_print(node n, FILE *f) {
    node_print_(n, f, 0);
}

void parser_init(Parser *p) {
    stack_init(&p->stack);
}

/* * * Helpers * * */

#define PARSER_BEGIN(parser, tokens)       \
    Parser   *p_ = parser;                 \
    token    *ts_ = tokens;                \
    stack_ptr st_ = stack_top(&p_->stack); \
    token     current_token = *(tokens);

#define PARSER_FAIL(result)        \
    stack_rewind(&p_->stack, st_); \
    return result;

#define PARSER_END        \
    *ts_ = current_token; \
    return PARSE_OK;

#define NEXT(token)                                            \
    p_->lex_result = token_next(&(token));                     \
    if (p_->lex_result != LEX_OK) { PARSER_FAIL(PARSE_ELEX); }

#define THEN_TOKEN(token_type, var) \
    THEN_TOKEN_(token_type);        \
    token var = current_token;

#define THEN_TOKEN_(token_type)                                                \
    NEXT(current_token);                                                       \
    p_->expected_token = (token_type);                                         \
    if (current_token.type != p_->expected_token) { PARSER_FAIL(PARSE_ETOK); }

#define THEN(parser, var) \
    THEN_(parser);        \
    node *var = p_->node;

#define THEN_(parser)                                    \
    {                                                    \
        parse_result result = PARSE(parser);             \
        if (result != PARSE_OK) { PARSER_FAIL(result); } \
    }

#define THEN_V(parser, var, ...)  \
    THEN_V_(parser, __VA_ARGS__); \
    node *var = p_->node;

#define THEN_V_(parser, ...)                                \
    {                                                       \
        parse_result result = PARSE_V(parser, __VA_ARGS__); \
        if (result != PARSE_OK) { PARSER_FAIL(result); }    \
    }

#define PARSE(parser)        parse_##parser(p, &current_token)
#define PARSE_V(parser, ...) parse_##parser(p, &current_token, __VA_ARGS__)

#define PARSER_ATOM(name, token_type, node_type)      \
    parse_result name(Parser *p, token *t) {          \
        PARSER_BEGIN(p, t);                           \
        THEN_TOKEN_(token_type);                      \
        p->node = stack_push_zeroed(&p->stack, node); \
        p->node->type = node_type;                    \
        p->node->token = current_token;               \
        PARSER_END;                                   \
    }

/* * * Parsers * * */

PARSER_ATOM(parse_ident, T_IDENT, N_IDENT)
PARSER_ATOM(parse_string, T_STRING, N_STRING)
PARSER_ATOM(parse_int, T_INT, N_INT)
PARSER_ATOM(parse_dec, T_DEC, N_DEC)

parse_result parse_assign(Parser *p, token *ts) {
    PARSER_BEGIN(p, ts);

    THEN_TOKEN_(T_LET);
    THEN(pattern, lhs);
    THEN_TOKEN_(T_EQ);
    THEN_V(expr, rhs, 0);
    THEN_TOKEN_(T_SEMI);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_ASSIGN;
    p->node->first_child = lhs;
    lhs->next_sibling = rhs;

    PARSER_END;
}

parse_result parse_tuple_or_parens(Parser *p, token *ts) {
    PARSER_BEGIN(p, ts);

    token peek;
    node *first_item = NULL, *last_item = NULL;

    THEN_TOKEN_(T_POPEN);

    while (true) {
        peek = current_token;
        NEXT(peek);

        switch (peek.type) {
        case T_PCLOSE:
            printf("close\n");
            current_token = peek;

            if (first_item && first_item == last_item) {
                p->node = last_item;
                PARSER_END;
            }

            p->node = stack_push_zeroed(&p->stack, node);
            p->node->type = N_TUPLE;

            if (first_item) p->node->first_child = first_item;

            PARSER_END;
        case T_COMMA:
            printf("comma\n");
            current_token = peek;

            if (!first_item) {
                p->expected_token = T_PCLOSE; // arbitrary
                PARSER_FAIL(PARSE_ETOK);
            }

            NEXT(peek);

            if (peek.type == T_PCLOSE) {
                current_token = peek;

                p->node = stack_push_zeroed(&p->stack, node);
                p->node->type = N_TUPLE;
                p->node->first_child = first_item;

                PARSER_END;
            }
        default:
            printf(F_TOKEN, FA_TOKEN(peek));

            if (last_item) {
                p->expected_token = T_COMMA;
                PARSER_FAIL(PARSE_ETOK);
            }
        }

        THEN_V(expr, item, EXPR_ALL);
        printf("expr\n");

        if (first_item) {
            last_item->next_sibling = item;
            last_item = item;
        } else {
            first_item = last_item = item;
        }
    }

    PARSER_END;
}

parse_result parse_spread(Parser *p, token *ts) {
    PARSER_BEGIN(p, ts);

    THEN_TOKEN_(T_ELLIPS);
    THEN_V(expr, expr, EXPR_ALL);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_SPREAD;
    p->node->first_child = expr;

    PARSER_END;
}

parse_result parse_unary(Parser *p, token *ts) {
    PARSER_BEGIN(p, ts);

    THEN_TOKEN(T_OP, op);
    THEN_V(expr, expr, ~EXPR_BINARY);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_UNARY;
    p->node->token = op;
    p->node->first_child = expr;

    PARSER_END;
}

parse_result parse_expr(Parser *p, token *ts, parse_expr_flags flags) {
    // TODO: Placeholder
    return parse_ident(p, ts);
}

parse_result parse_pattern(Parser *p, token *ts) {
    // TODO: Placeholder
    return PARSE_ETOK;
}

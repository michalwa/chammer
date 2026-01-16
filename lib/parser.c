#include "parser.h"

#include <stdarg.h>
#include <string.h>

#include "buffer.h"
#include "lexer.h"
#include "stack.h"
#include "utils.h"

struct opdef {
    char  name[OPERATOR_MAX_LEN + 1];
    int   precedence;
    assoc assoc;
};

const char *node_type_name(node_type value) {
    RETURN_ENUM_NAME(node_type, value, EACH_NODE_TYPE);
}

const char *parse_result_name(parse_result value) {
    RETURN_ENUM_NAME(parse_result, value, EACH_PARSE_RESULT);
}

const char *assoc_name(assoc value) {
    RETURN_ENUM_NAME(assoc, value, EACH_ASSOC);
}

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
        child->parent = parent;

        last_child = &(*last_child)->next_sibling;
    }

    va_end(args);
}

static inline void node_assign_parent(node *parent) {
    for (node *n = parent->first_child; n; n = n->next_sibling) n->parent = parent;
}

/*
 * Appends a node to a double-ended linked list of siblings
 */
static inline void node_double_ended_append(node **first, node **last, node *n) {
    if (*last) {
        (*last)->next_sibling = n;
        *last = n;
    } else {
        *first = *last = n;
    }
}

void parser_init(Parser *p) {
    stack_init(&p->stack);
    p->operators = calloc(MAX_OPERATORS, sizeof(opdef));
    p->operators_len = 0;
    p->node = NULL;
}

void parser_free(Parser *p) {
    stack_free(&p->stack);
    free(p->operators);
}

static opdef *parser_get_operator(Parser *p, const char *name, size_t name_len) {
    for (size_t i = 0; i < p->operators_len; i++)
        if (strlen(p->operators[i].name) == name_len
            && strncmp(p->operators[i].name, name, name_len) == 0)
            return &p->operators[i];

    return NULL;
}

void parser_define_operator(
    Parser *p, const char *name, size_t name_len, int precedence, assoc assoc
) {
    opdef *op = parser_get_operator(p, name, name_len);

    if (op) {
        fprintf(stderr, "WARN: redefinition of operator %.*s\n", (int)name_len, name);
    } else {
        if (p->operators_len >= MAX_OPERATORS) panic("`MAX_OPERATORS` limit reached");
        op = &p->operators[p->operators_len++];
    }

    if (name_len > OPERATOR_MAX_LEN) panic("`OPERATOR_MAX_LEN` exceeded\n");

    strncpy(op->name, name, name_len);
    op->precedence = precedence;
    op->assoc = assoc;
}

/*
 * A container for the discardable state of a parser routine
 *
 * This struct and associated macros and functions define a framework of
 * primitives, in terms of which parser routines are written
 */
typedef struct {
    Parser      *parser;
    stack_ptr    stack_begin;
    token       *token_stream;
    token        current_token;
    parse_result result;
} frame;

static inline frame begin(Parser *p, token *ts) {
    return (frame){
        .parser = p,
        .stack_begin = stack_top(&p->stack),
        .token_stream = ts,
        .current_token = *ts,
        .result = PARSE_OK,
    };
}

static inline parse_result discard(frame f) {
    stack_rewind(&f.parser->stack, f.stack_begin);
    return f.result;
}

#ifdef HAMMER_DEBUG
#define DISCARD(f) DO(return discard_checked(__FILE__, __LINE__, f))

static inline parse_result discard_checked(const char *file, int line, frame f) {
    if (f.result == PARSE_OK)
        panic_(file, line, "`DISCARD` called, but last parse result is `PARSE_OK`\n");

    return discard(f);
}
#else
#define DISCARD(f) DO(return discard(f))
#endif

#define COMMIT(f) DO(return commit(f))

static inline parse_result commit(frame f) {
    *f.token_stream = f.current_token;
    return PARSE_OK;
}

#define NEXT_TOKEN(f, t) DO(if (!next_token(&(f), &(t))) DISCARD(f))

static inline bool next_token(frame *f, token *t) {
    f->parser->lex_result = token_next(t);
    f->result = f->parser->lex_result == LEX_OK ? PARSE_OK : PARSE_ELEX;
    return f->result == PARSE_OK;
}

#define THEN_TOKEN(f, var, t) DO(THEN_TOKEN_(f, t); var = f.current_token)
#define THEN_TOKEN_(f, t)     DO(if (!expect_token(&(f), t)) DISCARD(f))

static inline bool expect_token(frame *f, token_type t) {
    if (next_token(f, &f->current_token)) {
        f->parser->expected_token = t;
        f->result = f->current_token.type == t ? PARSE_OK : PARSE_ETOK;
    }
    return f->result == PARSE_OK;
}

#define THEN(f, var, parser_fn)        DO(THEN_(f, parser_fn); var = f.parser->node)
#define THEN_(f, parser_fn)            DO(if (PARSE(f, parser_fn) != PARSE_OK) DISCARD(f))
#define THEN_V(f, var, parser_fn, ...) DO(THEN_V_(f, parser_fn, __VA_ARGS__); var = f.parser->node)
#define THEN_V_(f, parser_fn, ...)                                     \
    DO(if (PARSE_V(f, parser_fn, __VA_ARGS__) != PARSE_OK) DISCARD(f))

#define TRY(f, parser_fn)          (PARSE(f, parser_fn) == PARSE_OK)
#define TRY_V(f, parser_fn, ...)   (PARSE_V(f, parser_fn, __VA_ARGS__) == PARSE_OK)
#define PARSE(f, parser_fn)        (f.result = parser_fn(f.parser, &f.current_token))
#define PARSE_V(f, parser_fn, ...) (f.result = parser_fn(f.parser, &f.current_token, __VA_ARGS__))

parse_result parse_atom(Parser *p, token *ts, token_type t, node_type n) {
    frame f = begin(p, ts);
    THEN_TOKEN_(f, t);
    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = n;
    p->node->token = f.current_token;
    COMMIT(f);
}

inline parse_result parse_program(Parser *p, token *ts) {
    return parse_doblk_body(p, ts);
}

#define PARSER_ATOM(name, token_type, node_type)         \
    inline parse_result name(Parser *p, token *ts) {     \
        return parse_atom(p, ts, token_type, node_type); \
    }

PARSER_ATOM(parse_ident, T_IDENT, N_IDENT)
PARSER_ATOM(parse_string, T_STRING, N_STRING)
PARSER_ATOM(parse_int, T_INT, N_INT)
PARSER_ATOM(parse_dec, T_DEC, N_DEC)

#undef PARSER_ATOM

parse_result parse_stmt(Parser *p, token *ts, parse_stmt_flags flags) {
    frame f = begin(p, ts);

    if (flags & STMT_DOBIND && TRY(f, parse_dobind)) COMMIT(f);
    if (TRY(f, parse_assign)) COMMIT(f);
    if (TRY(f, parse_void)) COMMIT(f);

    DISCARD(f);
}

parse_result parse_assign(Parser *p, token *ts) {
    frame f = begin(p, ts);
    node *lhs, *rhs;

    THEN_TOKEN_(f, T_LET);
    THEN(f, lhs, parse_lhs_pattern);
    THEN_TOKEN_(f, T_EQ);
    THEN_V(f, rhs, parse_expr, EXPR_ALL);
    THEN_TOKEN_(f, T_SEMI);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_ASSIGN;
    node_add_children(p->node, lhs, rhs);

    COMMIT(f);
}

parse_result parse_dobind(Parser *p, token *ts) {
    frame f = begin(p, ts);
    node *lhs, *rhs;

    THEN_TOKEN_(f, T_LET);
    THEN(f, lhs, parse_lhs_pattern);
    THEN_TOKEN_(f, T_LARROW);
    THEN_V(f, rhs, parse_expr, EXPR_ALL);
    THEN_TOKEN_(f, T_SEMI);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_DOBIND;
    node_add_children(p->node, lhs, rhs);

    COMMIT(f);
}

parse_result parse_void(Parser *p, token *ts) {
    frame f = begin(p, ts);
    node *expr;

    THEN_V(f, expr, parse_expr, EXPR_ALL);
    THEN_TOKEN_(f, T_SEMI);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_VOID;
    node_add_children(p->node, expr);

    COMMIT(f);
}

parse_result parse_expr(Parser *p, token *ts, parse_expr_flags flags) {
    frame f = begin(p, ts);

    if (flags & EXPR_BINARY && TRY(f, parse_binary)) COMMIT(f);
    if (flags & EXPR_UNARY && TRY(f, parse_unary)) COMMIT(f);
    if (flags & EXPR_APPLY && TRY(f, parse_apply)) COMMIT(f);
    if (TRY(f, parse_tuple_or_parens)) COMMIT(f);
    if (TRY(f, parse_list)) COMMIT(f);
    if (TRY(f, parse_block)) COMMIT(f);
    if (TRY(f, parse_doblk)) COMMIT(f);
    if (TRY(f, parse_if)) COMMIT(f);
    if (TRY(f, parse_match)) COMMIT(f);
    if (TRY(f, parse_lambda)) COMMIT(f);
    if (TRY(f, parse_ident)) COMMIT(f);
    if (TRY(f, parse_string)) COMMIT(f);
    if (TRY(f, parse_int)) COMMIT(f);
    if (TRY(f, parse_dec)) COMMIT(f);

    DISCARD(f);
}

parse_result parse_tuple_or_parens(Parser *p, token *ts) {
    frame f = begin(p, ts);

    token peek;
    node *first_item = NULL, *last_item = NULL;
    node *item;

    THEN_TOKEN_(f, T_POPEN);

    while (true) {
        peek = f.current_token;
        NEXT_TOKEN(f, peek);

        switch (peek.type) {
        case T_PCLOSE:
            f.current_token = peek;

            if (first_item && first_item == last_item) {
                p->node = last_item;
                COMMIT(f);
            }

            p->node = stack_push_zeroed(&p->stack, node);
            p->node->type = N_TUPLE;

            if (first_item) {
                p->node->first_child = first_item;
                node_assign_parent(p->node);
            }

            COMMIT(f);
        case T_COMMA:
            f.current_token = peek;

            if (!first_item) {
                p->expected_token = T_PCLOSE; // arbitrary
                f.result = PARSE_ETOK;
                DISCARD(f);
            }

            NEXT_TOKEN(f, peek);

            if (peek.type == T_PCLOSE) {
                f.current_token = peek;

                p->node = stack_push_zeroed(&p->stack, node);
                p->node->type = N_TUPLE;
                p->node->first_child = first_item;
                node_assign_parent(p->node);

                COMMIT(f);
            }

            break;
        default:
            THEN_V(f, item, parse_expr, EXPR_ALL);
            node_double_ended_append(&first_item, &last_item, item);
        }
    }

    COMMIT(f);
}

parse_result parse_list(Parser *p, token *ts) {
    frame f = begin(p, ts);

    token peek;
    node *first_item = NULL, *last_item = NULL;

    THEN_TOKEN_(f, T_SOPEN);

    while (true) {
        peek = f.current_token;
        NEXT_TOKEN(f, peek);

        switch (peek.type) {
        case T_SCLOSE:
            f.current_token = peek;

            p->node = stack_push_zeroed(&p->stack, node);
            p->node->type = N_LIST;

            if (first_item) {
                p->node->first_child = first_item;
                node_assign_parent(p->node);
            }

            COMMIT(f);
        case T_COMMA:
            f.current_token = peek;

            if (!first_item) {
                p->expected_token = T_SCLOSE; // arbitrary
                f.result = PARSE_ETOK;
                DISCARD(f);
            }

            break;
        default:
            if (last_item) {
                p->expected_token = T_COMMA;
                f.result = PARSE_ETOK;
                DISCARD(f);
            }
        }

        node *item;

        THEN_V(f, item, parse_expr, EXPR_ALL);
        node_double_ended_append(&first_item, &last_item, item);
    }

    COMMIT(f);
}

parse_result parse_spread(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *expr;

    THEN_TOKEN_(f, T_ELLIPS);
    THEN_V(f, expr, parse_expr, EXPR_ALL);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_SPREAD;
    node_add_children(p->node, expr);

    COMMIT(f);
}

parse_result parse_block(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *first_child = NULL, *last_child = NULL;

    THEN_TOKEN_(f, T_COPEN);

    while (TRY_V(f, parse_stmt, ~STMT_DOBIND))
        node_double_ended_append(&first_child, &last_child, p->node);

    if (TRY_V(f, parse_expr, EXPR_ALL))
        node_double_ended_append(&first_child, &last_child, p->node);

    THEN_TOKEN_(f, T_CCLOSE);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_BLOCK;
    p->node->first_child = first_child;
    node_assign_parent(p->node);

    COMMIT(f);
}

parse_result parse_doblk(Parser *p, token *ts) {
    frame f = begin(p, ts);

    THEN_TOKEN_(f, T_DO);
    THEN_TOKEN_(f, T_COPEN);
    THEN_(f, parse_doblk_body);
    THEN_TOKEN_(f, T_CCLOSE);

    COMMIT(f);
}

parse_result parse_doblk_body(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *first_child = NULL, *last_child = NULL;

    while (TRY_V(f, parse_stmt, STMT_ALL))
        node_double_ended_append(&first_child, &last_child, p->node);

    if (TRY_V(f, parse_expr, EXPR_ALL))
        node_double_ended_append(&first_child, &last_child, p->node);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_DOBLK;
    p->node->first_child = first_child;
    node_assign_parent(p->node);

    COMMIT(f);
}

parse_result parse_if(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *cond, *then, *elze;

    THEN_TOKEN_(f, T_IF);
    THEN_V(f, cond, parse_expr, EXPR_ALL);
    THEN_TOKEN_(f, T_THEN);
    THEN_V(f, then, parse_expr, EXPR_ALL);
    THEN_TOKEN_(f, T_ELSE);
    THEN_V(f, elze, parse_expr, EXPR_ALL);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_IF;
    node_add_children(p->node, cond, then, elze);

    COMMIT(f);
}

parse_result parse_match(Parser *p, token *ts) {
    frame f = begin(p, ts);

    token peek;
    node *first_child = NULL, *last_child = NULL;
    node *scrutinee;

    THEN_TOKEN_(f, T_MATCH);
    THEN_V(f, scrutinee, parse_expr, EXPR_ALL);
    node_double_ended_append(&first_child, &last_child, scrutinee);

    while (true) {
        peek = f.current_token;
        NEXT_TOKEN(f, peek);

        switch (peek.type) {
        case T_CASE:
            f.current_token = peek;

            node *lhs, *rhs;
            THEN(f, lhs, parse_pattern);
            node_double_ended_append(&first_child, &last_child, lhs);
            THEN_V(f, rhs, parse_expr, EXPR_ALL);
            node_double_ended_append(&first_child, &last_child, rhs);

            break;
        case T_ELSE:
            f.current_token = peek;

            node *elze;
            THEN_V(f, elze, parse_expr, EXPR_ALL);
            node_double_ended_append(&first_child, &last_child, elze);

            break;
        default:
            if (!first_child) {
                p->expected_token = T_CASE;
                f.result = PARSE_ETOK;
                DISCARD(f);
            }

            goto done;
        }
    }

done:
    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_MATCH;
    p->node->first_child = first_child;
    node_assign_parent(p->node);

    COMMIT(f);
}

parse_result parse_lambda(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *first_child = NULL, *last_child = NULL;
    node *body;

    THEN_TOKEN_(f, T_BSLASH);

    while (TRY(f, parse_pattern)) node_double_ended_append(&first_child, &last_child, p->node);

    if (!first_child) DISCARD(f);

    THEN_TOKEN_(f, T_RARROW);
    THEN_V(f, body, parse_expr, EXPR_ALL);
    node_double_ended_append(&first_child, &last_child, body);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_LAMBDA;
    p->node->first_child = first_child;
    node_assign_parent(p->node);

    COMMIT(f);
}

parse_result parse_apply(Parser *p, token *ts) {
    frame f = begin(p, ts);

    node *first_child = NULL, *last_child = NULL;
    node *func;

    THEN_V(f, func, parse_expr, ~EXPR_APPLY & ~EXPR_BINARY & ~EXPR_UNARY);
    node_double_ended_append(&first_child, &last_child, func);

    while (TRY_V(f, parse_expr, ~EXPR_APPLY & ~EXPR_BINARY & ~EXPR_UNARY))
        node_double_ended_append(&first_child, &last_child, p->node);

    if (first_child == last_child) DISCARD(f);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_APPLY;
    p->node->first_child = first_child;
    node_assign_parent(p->node);

    COMMIT(f);
}

parse_result parse_unary(Parser *p, token *ts) {
    frame f = begin(p, ts);

    token op;
    node *expr;

    THEN_TOKEN(f, op, T_OP);
    THEN_V(f, expr, parse_expr, ~EXPR_BINARY);

    p->node = stack_push_zeroed(&p->stack, node);
    p->node->type = N_UNARY;
    p->node->token = op;
    node_add_children(p->node, expr);

    COMMIT(f);
}

parse_result parse_binary(Parser *p, token *ts) {
    frame f = begin(p, ts);

    token op_token;
    node *root, *rightmost = NULL, *rhs = NULL;
    int   last_precedence;

    THEN_V(f, root, parse_expr, ~EXPR_BINARY);

    while (true) {
        if (rhs) {
            op_token = f.current_token;
            if (token_next(&op_token) != LEX_OK || op_token.type != T_OP) break;
            f.current_token = op_token;
        } else {
            THEN_TOKEN(f, op_token, T_OP);
        }

        THEN_V(f, rhs, parse_expr, ~EXPR_BINARY);

        p->node = stack_push_zeroed(&p->stack, node);
        p->node->type = N_BINARY;
        p->node->token = op_token;

        opdef *op = parser_get_operator(p, op_token.str, op_token.len);
        int    precedence = op ? op->precedence : 0;

        if (!rightmost || precedence > last_precedence
            || precedence == last_precedence && op->assoc == ASSOC_LEFT) {
            node_add_children(p->node, root, rhs);
            rightmost = root = p->node;
        } else {
            node_add_children(p->node, rightmost->first_child->next_sibling, rhs);
            rightmost->first_child->next_sibling = p->node;
            p->node->parent = rightmost->first_child;
            rightmost = p->node;
        }

        last_precedence = precedence;
    }

    p->node = root;

    COMMIT(f);
}

parse_result parse_lhs_pattern(Parser *p, token *ts) {
    // TODO: Placeholder
    (void)p;
    (void)ts;
    return PARSE_ETOK;
}

parse_result parse_pattern(Parser *p, token *ts) {
    // TODO: Placeholder
    (void)p;
    (void)ts;
    return PARSE_ETOK;
}

#undef DISCARD
#undef COMMIT
#undef NEXT_TOKEN
#undef THEN_TOKEN
#undef THEN_TOKEN_
#undef THEN
#undef THEN_
#undef THEN_V
#undef THEN_V_
#undef TRY
#undef TRY_V
#undef PARSE
#undef PARSE_V

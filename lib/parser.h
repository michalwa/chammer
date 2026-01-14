#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#include "buffer.h"
#include "lexer.h"
#include "stack.h"
#include "string.h"
#include "utils.h"

#define NODE_TYPES                          \
    _(N_ASSIGN) /* assignment            */ \
    _(N_IDENT)  /* identifier expression */ \
    _(N_STRING) /* string literal        */ \
    _(N_INT)    /* integer literal       */ \
    _(N_DEC)    /* decimal literal       */ \
    _(N_TUPLE)  /* tuple literal         */ \
    _(N_LIST)   /* list literal          */ \
    _(N_SPREAD) /* list spread           */ \
    _(N_UNARY)  /* unary operation       */ \
    _(N_BINARY) /* binary operation      */ \
    _(N_APPLY)  /* function application  */ \
    _(N_IF)     /* if expression         */ \
    _(N_MATCH)  /* match expression      */ \
    _(N_LAMBDA) /* lambda expression     */ \
    _(N_BLOCK)  /* block expression      */ \
    _(N_DOBLK)  /* do-block expression   */ \
    _(N_DOBIND) /* monadic binding       */ \
    _(N_VOID)   /* expression statement  */ \
    _(N_PIDENT) /* identifier pattern    */ \
    _(N_PWILD)  /* wildcard pattern      */ \
    _(N_PAPPLY) /* function pattern      */ \
    _(N_PTUPLE) /* tuple pattern         */ \
    _(N_PLIST)  /* list pattern          */ \
    _(N_PLTAIL) /* list pattern tail     */ \
    _(N_PALIAS) /* alias pattern         */ \
    _(N_PCONST) /* const/expr pattern    */

#define _(name) name,
typedef enum { NODE_TYPES } node_type;
#undef _

#define NODE_FLAGS                            \
    _(NF_REC, 1)   /* recursive `N_ASSIGN' */ \
    _(NF_NAMED, 2) /* named `N_PLTAIL'     */

#define _(name, value) name = value,
typedef enum { NODE_FLAGS } node_flags;
#undef _

typedef struct node {
    node_type    type;
    node_flags   flags;
    struct node *first_child;
    struct node *next_sibling;
    struct node *parent;
    /*
     * For `N_IDENT', `N_STR', `N_INT', `N_DEC', `N_PIDENT'
     *   this is the full token that was parsed into the node
     * For `N_UNARY' and `N_BINARY' it is the `T_OP' token
     * For `N_PAPPLY' and `N_PALIAS' it is the `T_IDENT' token
     * For `N_PLTAIL' with the `NF_NAMED' flag it is the `T_IDENT' token
     */
    token token;
} node;

typedef enum parse_result {
    PARSE_OK = 0,
    PARSE_ELEX = 1,
    PARSE_ETOK = 2,
} parse_result;

typedef struct Parser {
    Stack stack;
    /*
     * Holds the root node in case of a successful `PARSE_OK' result
     */
    node *node;
    /*
     * Holds the lexer result in case of a `PARSE_ELEX' result
     */
    lex_result lex_result;
    /*
     * Holds the expected token type in case of a `PARSE_ETOK' result
     */
    token_type expected_token;
} Parser;

typedef enum parse_expr_flags {
    EXPR_ALL = -1,
    EXPR_BINARY = 1,
    EXPR_UNARY = 1 << 1,
    EXPR_APPLY = 1 << 2,
} parse_expr_flags;

typedef enum parse_stmt_flags {
    STMT_ALL = -1,
    STMT_DOBIND = 1,
} parse_stmt_flags;

#define node_add_children(parent, ...) node_add_children_(parent, ARGC(__VA_ARGS__), __VA_ARGS__)

const char *node_name(node);
void        node_print(node, Buffer *);
void        node_add_children_(node *parent, int n, ...);

void         parser_init(Parser *);
void         parser_free(Parser *);
parse_result parse(Parser *, token *);

parse_result parse_ident(Parser *, token *);
parse_result parse_string(Parser *, token *);
parse_result parse_int(Parser *, token *);
parse_result parse_dec(Parser *, token *);

parse_result parse_stmt(Parser *, token *, parse_stmt_flags);
parse_result parse_assign(Parser *, token *);
parse_result parse_dobind(Parser *, token *);
parse_result parse_void(Parser *, token *);

parse_result parse_expr(Parser *, token *, parse_expr_flags);
parse_result parse_tuple_or_parens(Parser *, token *);
parse_result parse_list(Parser *, token *);
parse_result parse_spread(Parser *, token *);
parse_result parse_block(Parser *, token *);
parse_result parse_doblk(Parser *, token *);
parse_result parse_doblk_body(Parser *, token *);
parse_result parse_if(Parser *, token *);
parse_result parse_match(Parser *, token *);
parse_result parse_lambda(Parser *, token *);
parse_result parse_apply(Parser *, token *);
parse_result parse_unary(Parser *, token *);
parse_result parse_binary(Parser *, token *);

/**
 * Patterns which are valid in the LHS position of an assignment or monadic
 * binding. Includes things like function definition patterns.
 */
parse_result parse_lhs_pattern(Parser *, token *);
parse_result parse_pattern(Parser *, token *);

#endif // PARSER_H_

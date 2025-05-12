#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#include "lexer.h"
#include "stack.h"
#include "string.h"

#define NODE_TYPES                        \
    _(ASSIGN) /* assignment            */ \
    _(IDENT)  /* identifier expression */ \
    _(STRING) /* string literal        */ \
    _(INT)    /* integer literal       */ \
    _(DEC)    /* decimal literal       */ \
    _(TUPLE)  /* tuple literal         */ \
    _(LIST)   /* list literal          */ \
    _(SPREAD) /* list spread           */ \
    _(UNARY)  /* unary operation       */ \
    _(BINARY) /* binary operation      */ \
    _(APPLY)  /* function application  */ \
    _(IF)     /* if expression         */ \
    _(MATCH)  /* match expression      */ \
    _(CASE)   /* match case            */ \
    _(LAMBDA) /* lambda expression     */ \
    _(BLOCK)  /* block expression      */ \
    _(DOBLK)  /* do-block expression   */ \
    _(DOBIND) /* monadic binding       */ \
    _(PIDENT) /* identifier pattern    */ \
    _(PWILD)  /* wildcard pattern      */ \
    _(PAPPLY) /* function pattern      */ \
    _(PTUPLE) /* tuple pattern         */ \
    _(PLIST)  /* list pattern          */ \
    _(PLTAIL) /* list pattern tail     */ \
    _(PALIAS) /* alias pattern         */ \
    _(PCONST) /* const/expr pattern    */

#define _(name) N_##name,
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
    /*
     * For `N_IDENT', `N_STR', `N_INT', `N_DEC', `N_PIDENT'
     *   this is the full token that was parsed into the node
     * For `N_UNARY' and `N_BINARY' it is the `T_OP' token
     * For `N_PAPPLY' and `N_PALIAS' it is the `T_IDENT' token
     * For `N_PLTAIL' with the `NF_NAMED' flag it is the `T_IDENT' token
     */
    token token;
} node;

typedef enum {
    PARSE_OK = 0,
    PARSE_ELEX = 1,
    PARSE_ETOK = 2,
} parse_result;

typedef struct {
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

typedef enum {
    EXPR_ALL = -1,
    EXPR_BINARY = 1,
} parse_expr_flags;

const char *node_name(node);
void        node_print(node, FILE *);

void         parser_init(Parser *);
parse_result parse(Parser *, token *);
parse_result parse_expr(Parser *, token *, parse_expr_flags);
parse_result parse_pattern(Parser *, token *);
parse_result parse_ident(Parser *, token *);
parse_result parse_string(Parser *, token *);
parse_result parse_int(Parser *, token *);
parse_result parse_dec(Parser *, token *);
parse_result parse_assign(Parser *, token *);
parse_result parse_tuple_or_parens(Parser *, token *);
parse_result parse_spread(Parser *, token *);
parse_result parse_unary(Parser *, token *);

#endif // PARSER_H_

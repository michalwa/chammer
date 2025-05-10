#ifndef PARSER_H_
#define PARSER_H_

#include <stdio.h>

#include "lexer.h"
#include "string.h"

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
    _(N_CASE)   /* match case            */ \
    _(N_LAMBDA) /* lambda expression     */ \
    _(N_BLOCK)  /* block expression      */ \
    _(N_DOBLK)  /* do-block expression   */ \
    _(N_DOBIND) /* monadic binding       */ \
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
    _(NF_REC, 1)   /* recursive `N_PAPPLY' */ \
    _(NF_NAMED, 2) /* named `N_PLTAIL'     */

#define _(name, value) name = value,
typedef enum { NODE_FLAGS } node_flags;
#undef _

typedef struct {
    node_type  type;
    node_flags flags;
    /*
     * For `N_IDENT', `N_STR', `N_INT', `N_DEC', `N_PIDENT'
     *   this is the full token that was parsed into the node
     * For `N_UNARY' and `N_BINARY' it is the `T_OP' token
     * For `N_PAPPLY' and `N_PALIAS' it is the `T_IDENT' token
     * For `N_PLTAIL' with the `NF_NAMED' flag it is the `T_IDENT' token
     */
    token token;
    /*
     * For `N_STR' this is the parsed/unescaped string
     */
    string str_parsed;
} node;

const char *node_name(node);
void        node_print(node, FILE *);

#endif // PARSER_H_

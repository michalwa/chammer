#ifndef _PARSER_H
#define _PARSER_H

#include "lexer.h"
#include "strings.h"

#define NODE_TYPES                          \
    _(N_ASSIGN) /* assignment            */ \
    _(N_IDENT)  /* identifier expression */ \
    _(N_STR)    /* string literal        */ \
    _(N_INT)    /* integer literal       */ \
    _(N_DEC)    /* decimal literal       */ \
    _(N_TUPLE)  /* tuple literal         */ \
    _(N_LIST)   /* list literal          */ \
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

typedef struct {
    node_type type;
    /*
     * For `N_IDENT', `N_STR', `N_INT', `N_DEC', `N_PIDENT'
     *   this is the full token that was parsed into the node
     * For `N_UNARY' and `N_BINARY' it is the `T_OP' token
     * For `N_PALIAS' it is the `T_IDENT' token
     * For `N_PLTAIL' it is the `T_IDENT' token if present, or `T_ELLIPS' otherwise
     */
    token token;
    /*
     * For `N_STR' this is the parsed/unescaped string
     */
    str str_parsed;
} node;

typedef struct {
    size_t parent;
    size_t child;
} relation;

typedef struct {
    node  *nodes;
    size_t nodes_capacity;
    size_t nodes_cursor;

    relation *rels;
    size_t rels_capacity;
    size_t rels_cursor;
} SyntaxTree;

#endif // _PARSER_H

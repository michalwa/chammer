#ifndef HAMMER_AST_H_
#define HAMMER_AST_H_

#include "buffer.h"
#include "lexer.h"

#define EACH_NODE_TYPE(_)                   \
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

#define ENUM_MEMBER(name) name,
typedef enum { EACH_NODE_TYPE(ENUM_MEMBER) } node_type;
#undef ENUM_MEMBER

#define EACH_NODE_FLAG(_)                                        \
    /* _(name, node_type, value) */                              \
    _(NF_REC, N_PAPPLY, 1)   /* recursive function definition */ \
    _(NF_NAMED, N_PLTAIL, 1) /* named/captured list tail     */

#define ENUM_MEMBER(name, node_type, value) name = value,
typedef enum { EACH_NODE_FLAG(ENUM_MEMBER) } node_flags;
#undef ENUM_MEMBER

typedef struct node node;
struct node {
    node_type  type;
    node_flags flags;
    node      *first_child;
    node      *next_sibling;
    /*
     * For `N_IDENT`, `N_STR`, `N_INT`, `N_DEC`, `N_PIDENT`
     *   this is the full token that was parsed into the node
     * For `N_UNARY` and `N_BINARY` it is the `T_OP` token
     * For `N_PAPPLY` and `N_PALIAS` it is the `T_IDENT` token
     * For `N_PLTAIL` with the `NF_NAMED` flag it is the `T_IDENT` token
     */
    token      token;
};

const char *node_type_name(node_type);

#define node_add_children(parent, ...) node_add_children_(parent, ARGC(__VA_ARGS__), __VA_ARGS__)

bool node_has_token(node n);
void node_print(node, Buffer *);
void node_add_children_(node *parent, int n, ...);

#endif // HAMMER_AST_H_

#ifndef _LEXER_H
#define _LEXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define TOKEN_TYPES         \
    _(T_IDENT)              \
    _(T_OP)                 \
    _(T_INT)                \
    _(T_LET)    /* let   */ \
    _(T_REC)    /* rec   */ \
    _(T_MATCH)  /* match */ \
    _(T_CASE)   /* case  */ \
    _(T_THEN)   /* then  */ \
    _(T_EQ)     /* =     */ \
    _(T_RARROW) /* ->    */ \
    _(T_SOPEN)  /* [     */ \
    _(T_SCLOSE) /* ]     */ \
    _(T_ELLIPS) /* ...   */ \
    _(T_COMMA)  /* ,     */ \
    _(T_SEMI)   /* ;     */ \
    _(T_POPEN)  /* (     */ \
    _(T_PCLOSE) /* )     */ \

#define _(name) name,
typedef enum { TOKEN_TYPES } token_type;
#undef _

typedef struct {
    token_type  type;
    const char *str;
    size_t      len;
} token;

typedef struct {
    uint16_t line, col;
} loc;

void token_begin(token *t, const char *buffer);
bool token_next(token *t);
bool token_eq(token a, token b);
loc token_loc(token t, const char *buffer);
const char *token_name(token t);

#endif // _LEXER_H

#ifndef LEXER_H_
#define LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define TOKEN_TYPES                 \
    _(BCOMM)  /* block comment   */ \
    _(LCOMM)  /* line comment    */ \
    _(LET)    /* `let'           */ \
    _(DO)     /* `do'            */ \
    _(IF)     /* `if'            */ \
    _(THEN)   /* `then'          */ \
    _(ELSE)   /* `else'          */ \
    _(MATCH)  /* `match'         */ \
    _(CASE)   /* `case'          */ \
    _(REC)    /* `rec'           */ \
    _(IDENT)  /* identifier      */ \
    _(UNDER)  /* `_'             */ \
    _(INFIX)  /* infix ident     */ \
    _(STRING) /* string literal  */ \
    _(INT)    /* integer literal */ \
    _(DEC)    /* decimal literal */ \
    _(OP)     /* operator        */ \
    _(EQ)     /* `='             */ \
    _(LARROW) /* `<-'            */ \
    _(RARROW) /* `->'            */ \
    _(ELLIPS) /* `...'           */ \
    _(BSLASH) /* `\`             */ \
    _(POPEN)  /* `('             */ \
    _(PCLOSE) /* `)'             */ \
    _(SOPEN)  /* `['             */ \
    _(SCLOSE) /* `]'             */ \
    _(COPEN)  /* `{'             */ \
    _(CCLOSE) /* `}'             */ \
    _(COMMA)  /* `,'             */ \
    _(SEMI)   /* `;'             */

#define _(name) T_##name,
typedef enum { TOKEN_TYPES } token_type;
#undef _

#define F_TOKEN         "%s `%.*s'"
#define FA_TOKEN(token) token_name(token), (int)(token).len, (token).str

typedef struct {
    token_type  type;
    const char *str;
    size_t      len;
} token;

typedef enum {
    LEX_NONE = 0,
    LEX_OK,
    LEX_EEOI, // Unexpected end of input
    LEX_ENUM, // Malformed number
} lex_result;

typedef struct {
    uint16_t line, col;
} loc;

void        token_begin(token *, const char *);
lex_result  token_next(token *);
bool        token_eq(token, token);
loc         token_loc(token, const char *);
const char *token_name(token);

#endif // LEXER_H_

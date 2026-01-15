#ifndef LEXER_H_
#define LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define TOKEN_TYPES                   \
    _(T_BCOMM)  /* block comment   */ \
    _(T_LCOMM)  /* line comment    */ \
    _(T_LET)    /* `let`           */ \
    _(T_DO)     /* `do`            */ \
    _(T_IF)     /* `if`            */ \
    _(T_THEN)   /* `then`          */ \
    _(T_ELSE)   /* `else`          */ \
    _(T_MATCH)  /* `match`         */ \
    _(T_CASE)   /* `case`          */ \
    _(T_REC)    /* `rec`           */ \
    _(T_IDENT)  /* identifier      */ \
    _(T_UNDER)  /* `_`             */ \
    _(T_INFIX)  /* infix ident     */ \
    _(T_STRING) /* string literal  */ \
    _(T_INT)    /* integer literal */ \
    _(T_DEC)    /* decimal literal */ \
    _(T_OP)     /* operator        */ \
    _(T_EQ)     /* `=`             */ \
    _(T_LARROW) /* `<-`            */ \
    _(T_RARROW) /* `->`            */ \
    _(T_ELLIPS) /* `...`           */ \
    _(T_BSLASH) /* `\`             */ \
    _(T_POPEN)  /* `(`             */ \
    _(T_PCLOSE) /* `)`             */ \
    _(T_SOPEN)  /* `[`             */ \
    _(T_SCLOSE) /* `]`             */ \
    _(T_COPEN)  /* `{`             */ \
    _(T_CCLOSE) /* `}`             */ \
    _(T_COMMA)  /* `,`             */ \
    _(T_SEMI)   /* `;`             */

#define _(name) name,
typedef enum { TOKEN_TYPES } token_type;
#undef _

#define F_TOKEN         "%s `%.*s`"
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

#ifndef HAMMER_LEXER_H_
#define HAMMER_LEXER_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "string.h"

#define EACH_TOKEN_TYPE(_)            \
    _(T_BCOMM)  /* block comment   */ \
    _(T_LCOMM)  /* line comment    */ \
    _(T_LET)    /* `let`           */ \
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
    _(T_AT)     /* `@`             */ \
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

#define ENUM_MEMBER(name) name,
typedef enum { EACH_TOKEN_TYPE(ENUM_MEMBER) } token_type;
#undef ENUM_MEMBER

#define F_TOKEN "%s `%.*s`"
#define FA_TOKEN(token)                                                                      \
    ((token).len ? token_type_name((token).type) : "(begin)"), (int)(token).len, (token).str

typedef struct {
    token_type  type;
    const char *str;
    size_t      len;
} token;

#define EACH_LEX_RESULT(_)                    \
    _(LEX_NONE)                               \
    _(LEX_OK)                                 \
    _(LEX_EEOI) /* unexpected end of input */ \
    _(LEX_ENUM) /* malformed number */

#define ENUM_MEMBER(name) name,
typedef enum { EACH_LEX_RESULT(ENUM_MEMBER) } lex_result;
#undef ENUM_MEMBER

typedef enum {
    LEX_ALL = -1,
    LEX_COMMENTS = 1,
} lex_flags;

typedef struct {
    uint16_t line, col;
} loc;

const char *token_type_name(token_type);
const char *lex_result_name(lex_result);

void       token_begin(token *, const char *);
lex_result token_next(token *, lex_flags);
string     token_string(token);
string     token_ident(token);
loc        token_loc(token, const char *);
bool       token_is_comment(token);
bool       token_is_binary_op(token);

#endif // HAMMER_LEXER_H_

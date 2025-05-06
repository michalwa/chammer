#ifndef _LEXER_H
#define _LEXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t     type;
    const char *str;
    size_t      len;
} token;

#define T_IDENT   1
#define T_OP      2
#define T_INT     3
#define T_LET     4 // let
#define T_REC     5 // rec
#define T_MATCH   6 // match
#define T_CASE    7 // case
#define T_THEN    8 // then
#define T_EQ      9 // =
#define T_RARROW 10 // ->
#define T_SOPEN  11 // [
#define T_SCLOSE 12 // ]
#define T_ELLIPS 13 // ...
#define T_COMMA  14 // ,
#define T_SEMI   15 // ;
#define T_POPEN  16 // (
#define T_PCLOSE 17 // )

bool next_token(const char *buffer, token *t);
bool token_eq(token a, token b);

#endif // _LEXER_H

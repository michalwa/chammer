#ifndef HAMMER_PARSER_H_
#define HAMMER_PARSER_H_

#include "arena.h"
#include "ast.h"
#include "lexer.h"

#define MAX_OPERATORS    0x100
#define OPERATOR_MAX_LEN 8

#define EACH_PARSE_RESULT(_) \
    _(PARSE_OK)              \
    _(PARSE_LEX_ERROR)       \
    _(PARSE_EXPECTED_TOKEN)  \
    _(PARSE_LEFTOVER_TOKENS)

#define ENUM_MEMBER(name) name,
typedef enum { EACH_PARSE_RESULT(ENUM_MEMBER) } parse_result;
#undef ENUM_MEMBER

#define EACH_ASSOC(_) \
    _(ASSOC_LEFT)     \
    _(ASSOC_RIGHT)

#define ENUM_MEMBER(name) name,
typedef enum { EACH_ASSOC(ENUM_MEMBER) } assoc;
#undef ENUM_MEMBER

typedef struct opdef opdef;

typedef struct {
    opdef     *operators;
    size_t     operators_len;
    Arena      nodes;
    /*
     * Holds the root node in case of a successful `PARSE_OK` result
     */
    node      *node;
    /*
     * Holds the lexer result in case of a `PARSE_LEX_ERROR` result
     */
    lex_result lex_result;
    /*
     * Holds the expected token type in case of a `PARSE_EXPECTED_TOKEN` result
     */
    token_type expected_token;
} Parser;

typedef enum {
    EXPR_ALL = -1,
    EXPR_BINARY = 1,
    EXPR_UNARY = 1 << 1,
    EXPR_APPLY = 1 << 2,
} parse_expr_flags;

typedef enum {
    PAT_ALL = -1,
    PAT_APPLY = 1,
} parse_pattern_flags;

const char *parse_result_name(parse_result);
const char *assoc_name(assoc);

void parser_init(Parser *);
void parser_free(Parser *);
void parser_reset(Parser *);
void parser_define_operator(Parser *, const char *, size_t, int precedence, assoc);

parse_result parse_program(Parser *, token *);
parse_result parse_ident(Parser *, token *);
parse_result parse_string(Parser *, token *);
parse_result parse_int(Parser *, token *);
parse_result parse_dec(Parser *, token *);
parse_result parse_stmt(Parser *, token *);
parse_result parse_assign(Parser *, token *);
parse_result parse_bind(Parser *, token *);
parse_result parse_void(Parser *, token *);
parse_result parse_expr(Parser *, token *, parse_expr_flags);
parse_result parse_tuple_or_parens(Parser *, token *);
parse_result parse_list(Parser *, token *);
parse_result parse_list_or_tuple_item(Parser *, token *);
parse_result parse_spread(Parser *, token *);
parse_result parse_block(Parser *, token *);
parse_result parse_block_body(Parser *, token *);
parse_result parse_if(Parser *, token *);
parse_result parse_match(Parser *, token *);
parse_result parse_lambda(Parser *, token *);
parse_result parse_apply(Parser *, token *);
parse_result parse_unary(Parser *, token *);
parse_result parse_binary(Parser *, token *);
parse_result parse_pattern(Parser *, token *, parse_pattern_flags);
parse_result parse_palias(Parser *, token *);
parse_result parse_papply(Parser *, token *);
parse_result parse_ptuple_or_parens(Parser *, token *);
parse_result parse_plist(Parser *, token *);
parse_result parse_plist_item(Parser *, token *);
parse_result parse_pltail(Parser *, token *);
parse_result parse_pident(Parser *, token *);
parse_result parse_pwild(Parser *, token *);
parse_result parse_pconst(Parser *, token *);

#endif // HAMMER_PARSER_H_

#include "lexer.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *str;
    uint8_t     type;
} keyword;

static const char *OP_CHARSET = "~!@#$%^&*-+=|:<>./?";

static const keyword KEYWORDS[] = {
    { "let",   T_LET    },
    { "rec",   T_REC    },
    { "match", T_MATCH  },
    { "case",  T_CASE   },
    { "then",  T_THEN   },
};

static const keyword KEYWORD_OPS[] = {
    { "=",     T_EQ     },
    { "->",    T_RARROW },
    { "...",   T_ELLIPS },
};

static const keyword KEYWORD_CHARS[] = {
    { "(",     T_POPEN  },
    { ")",     T_PCLOSE },
    { "[",     T_SOPEN  },
    { "]",     T_SCLOSE },
    { ",",     T_COMMA  },
    { ";",     T_SEMI   },
};

static bool find_keyword(const keyword *keywords, size_t num_keywords, token *t) {
    for (size_t i = 0; i < num_keywords; i++) {
        if (strncmp(keywords[i].str, t->str, t->len) == 0) {
            t->type = keywords[i].type;
            return true;
        }
    }

    return false;
}

static bool next_ident_or_keyword(token *t) {
    if (!isalpha(*t->str) && *t->str != '_') return false;

    t->len = 0;
    for (const char *c = t->str; isalnum(*c); c++)
        t->len++;

    t->type = T_IDENT;
    find_keyword(KEYWORDS, sizeof(KEYWORDS) / sizeof(keyword), t);

    return true;
}

static bool next_op(token *t) {
    if (!strchr(OP_CHARSET, *t->str)) return false;

    t->len = 0;
    for (const char *c = t->str; strchr(OP_CHARSET, *c); c++)
        t->len++;

    t->type = T_OP;
    find_keyword(KEYWORD_OPS, sizeof(KEYWORD_OPS) / sizeof(keyword), t);

    return true;
}

static bool next_int(token *t) {
    if (!isdigit(*t->str)) return false;

    t->len = 0;
    for (const char *c = t->str; isdigit(*c); c++)
        t->len++;

    t->type = T_INT;

    return true;
}

static bool next_char(token *t) {
    t->len = 1;
    return find_keyword(KEYWORD_CHARS, sizeof(KEYWORD_CHARS) / sizeof(keyword), t);
}

bool next_token(const char *buffer, token *t) {
    if (!t->str) {
        t->str = buffer;
        t->len = 0;
    }

    t->str += t->len;

    while (isspace(*t->str)) t->str++;

    return next_ident_or_keyword(t)
        || next_op(t)
        || next_int(t)
        || next_char(t);
}

bool token_eq(token a, token b) {
    return a.type == b.type
        && a.len == b.len
        && strncmp(a.str, b.str, a.len) == 0;
}

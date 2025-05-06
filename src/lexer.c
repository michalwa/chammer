#include "lexer.h"
#include <ctype.h>
#include <string.h>

typedef struct {
    const char *str;
    uint8_t     type;
} keyword;

#define _(name) [name] = #name,
static const char *TOKEN_NAMES[] = { TOKEN_TYPES };
#undef _

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

static bool token_find_keyword(token *t, const keyword *keywords, size_t num_keywords) {
    for (size_t i = 0; i < num_keywords; i++) {
        if (strncmp(keywords[i].str, t->str, t->len) == 0) {
            t->type = keywords[i].type;
            return true;
        }
    }

    return false;
}

static bool token_next_word(token *t) {
    if (!isalpha(*t->str) && *t->str != '_') return false;

    t->len = 0;
    for (const char *c = t->str; isalnum(*c); c++)
        t->len++;

    t->type = T_IDENT;
    token_find_keyword(t, KEYWORDS, sizeof(KEYWORDS) / sizeof(keyword));

    return true;
}

static bool token_next_op(token *t) {
    if (!strchr(OP_CHARSET, *t->str)) return false;

    t->len = 0;
    for (const char *c = t->str; strchr(OP_CHARSET, *c); c++)
        t->len++;

    t->type = T_OP;
    token_find_keyword(t, KEYWORD_OPS, sizeof(KEYWORD_OPS) / sizeof(keyword));

    return true;
}

static bool token_next_int(token *t) {
    if (!isdigit(*t->str)) return false;

    t->len = 0;
    for (const char *c = t->str; isdigit(*c); c++)
        t->len++;

    t->type = T_INT;

    return true;
}

static bool token_next_char(token *t) {
    t->len = 1;

    return token_find_keyword(t, KEYWORD_CHARS, sizeof(KEYWORD_CHARS) / sizeof(keyword));
}

inline void token_begin(token *t, const char *buffer) {
    t->str = buffer;
    t->len = 0;
}

bool token_next(token *t) {
    t->str += t->len;

    while (isspace(*t->str)) t->str++;

    return token_next_word(t)
        || token_next_op(t)
        || token_next_int(t)
        || token_next_char(t);
}

inline bool token_eq(token a, token b) {
    return a.type == b.type
        && a.len == b.len
        && strncmp(a.str, b.str, a.len) == 0;
}

loc token_loc(token t, const char *buffer) {
    loc l = {0};

    for (; buffer < t.str; buffer++) {
        l.col++;

        if (*buffer == '\n') {
            l.line++;
            l.col = 0;
        }
    }

    return l;
}

const char *token_name(token t) {
    return TOKEN_NAMES[t.type];
}

#include "lexer.h"

#include <ctype.h>
#include <string.h>

#include "utils.h"

typedef struct {
    const char *str;
    token_type  type;
} keyword;

static const char *OP_CHARSET = "~!@#$%^&*-+=|:<>./?";

static const keyword KEYWORDS[] = {
    { "let",   T_LET   },
    { "do",    T_DO    },
    { "if",    T_IF    },
    { "then",  T_THEN  },
    { "else",  T_ELSE  },
    { "match", T_MATCH },
    { "case",  T_CASE  },
    { "rec",   T_REC   },
    { "_",     T_UNDER },
};

static const keyword KEYWORD_GLYPHS[] = {
    { "=",   T_EQ     },
    { "<-",  T_LARROW },
    { "->",  T_RARROW },
    { "...", T_ELLIPS },
};

static const keyword KEYWORD_CHARS[] = {
    { "\\", T_BSLASH },
    { "(",  T_POPEN  },
    { ")",  T_PCLOSE },
    { "[",  T_SOPEN  },
    { "]",  T_SCLOSE },
    { "{",  T_COPEN  },
    { "}",  T_CCLOSE },
    { ",",  T_COMMA  },
    { ";",  T_SEMI   },
};

const char *token_type_name(token_type value) {
    RETURN_ENUM_NAME(token_type, value, EACH_TOKEN_TYPE);
}

const char *lex_result_name(lex_result value) {
    RETURN_ENUM_NAME(lex_result, value, EACH_LEX_RESULT);
}

static lex_result token_find_keyword(token *t, const keyword *keywords, size_t num_keywords) {
    for (size_t i = 0; i < num_keywords; i++) {
        if (strncmp(keywords[i].str, t->str, t->len) == 0) {
            t->type = keywords[i].type;
            return LEX_OK;
        }
    }

    return LEX_NONE;
}

static lex_result token_next_block_comment(token *t) {
    if (strncmp(t->str, "{-", 2) != 0) return LEX_NONE;

    t->len = 2;
    // Points at the last character of where the block would end
    // to check for the null byte
    const char *c = t->str + t->len + 1;

    while (*c && strncmp(t->str + t->len, "-}", 2) != 0) {
        t->len++;
        c++;
    }

    if (!*c) return LEX_EEOI;

    t->len += 2;
    t->type = T_BCOMM;
    return LEX_OK;
}

static lex_result token_next_line_comment(token *t) {
    if (strncmp(t->str, "--", 2) != 0) return LEX_NONE;

    t->len = 2;
    while (t->str[t->len] != 0 && !strchr("\n\r", t->str[t->len])) t->len++;

    t->type = T_LCOMM;

    return LEX_OK;
}

static lex_result token_next_string(token *t) {
    if (*t->str != '"') return LEX_NONE;

    bool escape = false;

    while (1) {
        switch (t->str[++t->len]) {
        case '\\': escape = true; continue;
        case '"':
            if (!escape) goto end;
            break;
        case '\0': return LEX_EEOI;
        }

        escape = false;
    }

end:
    t->len++;
    t->type = T_STRING;
    return LEX_OK;
}

static lex_result token_next_word(token *t) {
    if (!isalpha(*t->str) && *t->str != '_') return LEX_NONE;

    t->len = 0;
    while (isalnum(t->str[t->len])) t->len++;

    t->type = T_IDENT;
    token_find_keyword(t, KEYWORDS, sizeof(KEYWORDS) / sizeof(keyword));

    return LEX_OK;
}

static lex_result token_next_glyph(token *t) {
    if (!strchr(OP_CHARSET, *t->str)) return LEX_NONE;

    t->len = 0;
    while (strchr(OP_CHARSET, t->str[t->len])) t->len++;

    t->type = T_OP;
    token_find_keyword(t, KEYWORD_GLYPHS, sizeof(KEYWORD_GLYPHS) / sizeof(keyword));

    return LEX_OK;
}

static lex_result token_next_infix(token *t) {
    if (*t->str != '`') return LEX_NONE;

    t->len = 1;
    while (isalnum(t->str[t->len])) t->len++;

    // NOTE: The original version requires infix idents to terminate with a backtick,
    // but it's actually pretty pointless, aside from being consistent with Haskell
    // and maybe friendlier towards reusing syntax highlighting of other languages.

    t->type = T_INFIX;
    return LEX_OK;
}

static lex_result token_next_number(token *t) {
    if (!isdigit(*t->str)) return LEX_NONE;

    t->len = 0;
    while (isdigit(t->str[t->len])) t->len++;

    t->type = T_INT;

    if (t->str[t->len] == '.') {
        t->type = T_DEC;

        t->len++;
        while (isdigit(t->str[t->len])) t->len++;

        if (t->str[t->len - 1] == '.') return LEX_ENUM;
    }

    return LEX_OK;
}

static inline lex_result token_next_char(token *t) {
    t->len = 1;

    return token_find_keyword(t, KEYWORD_CHARS, sizeof(KEYWORD_CHARS) / sizeof(keyword));
}

inline void token_begin(token *t, const char *buffer) {
    t->str = buffer;
    t->len = 0;
}

lex_result token_next(token *t) {
    t->str += t->len;
    while (isspace(*t->str)) t->str++;

    if (!*t->str) return LEX_NONE;

    lex_result result;

    if ((result = token_next_block_comment(t)) != LEX_NONE) return result;
    if ((result = token_next_line_comment(t)) != LEX_NONE) return result;
    if ((result = token_next_string(t)) != LEX_NONE) return result;
    if ((result = token_next_word(t)) != LEX_NONE) return result;
    if ((result = token_next_glyph(t)) != LEX_NONE) return result;
    if ((result = token_next_char(t)) != LEX_NONE) return result;
    if ((result = token_next_infix(t)) != LEX_NONE) return result;
    if ((result = token_next_number(t)) != LEX_NONE) return result;

    return LEX_NONE;
}

inline bool token_eq(token a, token b) {
    return a.type == b.type && a.len == b.len && strncmp(a.str, b.str, a.len) == 0;
}

loc token_loc(token t, const char *buffer) {
    loc l = { 0 };

    for (; buffer < t.str; buffer++) {
        l.col++;

        if (*buffer == '\n') {
            l.line++;
            l.col = 0;
        }
    }

    return l;
}

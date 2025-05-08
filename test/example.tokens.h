#include "../lib/lexer.h"

#define EXAMPLE_TOKENS             \
    _(T_LET, "let")                \
    _(T_REC, "rec")                \
    _(T_IDENT, "map")              \
    _(T_IDENT, "f")                \
    _(T_BCOMM, "{- comment -}")    \
    _(T_IDENT, "xs")               \
    _(T_EQ, "=")                   \
    _(T_MATCH, "match")            \
    _(T_IDENT, "xs")               \
    _(T_LCOMM, "-- comment")       \
    _(T_CASE, "case")              \
    _(T_SOPEN, "[")                \
    _(T_SCLOSE, "]")               \
    _(T_THEN, "then")              \
    _(T_SOPEN, "[")                \
    _(T_SCLOSE, "]")               \
    _(T_CASE, "case")              \
    _(T_SOPEN, "[")                \
    _(T_IDENT, "x")                \
    _(T_COMMA, ",")                \
    _(T_ELLIPS, "...")             \
    _(T_IDENT, "xs")               \
    _(T_SCLOSE, "]")               \
    _(T_THEN, "then")              \
    _(T_SOPEN, "[")                \
    _(T_IDENT, "f")                \
    _(T_IDENT, "x")                \
    _(T_COMMA, ",")                \
    _(T_ELLIPS, "...")             \
    _(T_IDENT, "map")              \
    _(T_IDENT, "f")                \
    _(T_IDENT, "xs")               \
    _(T_SCLOSE, "]")               \
    _(T_SEMI, ";")                 \
    _(T_IDENT, "print")            \
    _(T_OP, "$")                   \
    _(T_IDENT, "map")              \
    _(T_POPEN, "(")                \
    _(T_IDENT, "x")                \
    _(T_RARROW, "->")              \
    _(T_IDENT, "x")                \
    _(T_OP, "+")                   \
    _(T_INT, "1")                  \
    _(T_PCLOSE, ")")               \
    _(T_SOPEN, "[")                \
    _(T_INT, "1")                  \
    _(T_COMMA, ",")                \
    _(T_INT, "2")                  \
    _(T_INFIX, "`mod")             \
    _(T_INT, "3")                  \
    _(T_COMMA, ",")                \
    _(T_DEC, "3.14")               \
    _(T_COMMA, ",")                \
    _(T_STRING, "\"string \\\"\"") \
    _(T_SCLOSE, "]")

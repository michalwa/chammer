#include "../lib/parser.h"

/*
 * An X-macro describing the syntax tree of the example source file
 *
 * Variable macros:
 *   `_BEGIN(<token_type>[, <token_type>, <token_str>[, <flags>[, <str_parsed>]]])'
 *   `_END'
 */
#define EXAMPLE_TREE                                                                 \
    _BEGIN(N_DOBLK)                                                                  \
        _BEGIN(N_ASSIGN)                                                             \
            _BEGIN(N_PAPPLY, T_IDENT, "map", NF_REC)                                 \
                _ATOM(N_PIDENT, T_IDENT, "f")                                        \
                _ATOM(N_PIDENT, T_IDENT, "xs")                                       \
            _END                                                                     \
            _BEGIN(N_MATCH)                                                          \
                _ATOM(N_IDENT, T_IDENT, "xs")                                        \
                _BEGIN(N_CASE)                                                       \
                    _ATOM(N_PLIST)                                                   \
                    _ATOM(N_LIST)                                                    \
                _END                                                                 \
                _BEGIN(N_CASE)                                                       \
                    _BEGIN(N_PLIST)                                                  \
                        _ATOM(N_PIDENT, T_IDENT, "x")                                \
                        _ATOM(N_PLTAIL, T_IDENT, "xs", NF_NAMED)                     \
                    _END                                                             \
                    _BEGIN(N_LIST)                                                   \
                        _BEGIN(N_APPLY)                                              \
                            _ATOM(N_IDENT, T_IDENT, "f")                             \
                            _ATOM(N_IDENT, T_IDENT, "x")                             \
                        _END                                                         \
                        _BEGIN(N_SPREAD)                                             \
                            _BEGIN(N_APPLY)                                          \
                                _ATOM(N_IDENT, T_IDENT, "map")                       \
                                _ATOM(N_IDENT, T_IDENT, "f")                         \
                                _ATOM(N_IDENT, T_IDENT, "xs")                        \
                            _END                                                     \
                        _END                                                         \
                    _END                                                             \
                _END                                                                 \
            _END                                                                     \
        _END                                                                         \
        _BEGIN(N_BINARY, T_OP, "$")                                                  \
            _ATOM(N_IDENT, T_IDENT, "print")                                         \
            _BEGIN(N_APPLY)                                                          \
                _ATOM(N_IDENT, T_IDENT, "map")                                       \
                _BEGIN(N_LAMBDA)                                                     \
                    _ATOM(N_PIDENT, T_IDENT, "x")                                    \
                    _BEGIN(N_BINARY, T_OP, "+")                                      \
                        _ATOM(N_IDENT, T_IDENT, "x")                                 \
                        _ATOM(N_INT, T_INT, "1")                                     \
                    _END                                                             \
                    _BEGIN(N_LIST)                                                   \
                        _ATOM(N_INT, T_INT, "1")                                     \
                        _BEGIN(N_BINARY, T_INFIX, "`mod")                            \
                            _ATOM(N_INT, T_INT, "2")                                 \
                            _ATOM(N_INT, T_INT, "3")                                 \
                        _END                                                         \
                        _ATOM(N_DEC, T_DEC, "3.14")                                  \
                        _ATOM(N_STRING, T_STRING, "\"string \\\"\"", 0, "string \"") \
                    _END                                                             \
                _END                                                                 \
            _END                                                                     \
        _END                                                                         \
    _END

#define _ATOM(...)      \
    _BEGIN(__VA_ARGS__) \
    _END

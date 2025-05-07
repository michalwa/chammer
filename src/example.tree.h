#include "parser.h"

/*
 * An X-macro describing the syntax tree of the example source file
 *
 * Variable macros:
 *   `_BEGIN(<token_type>, [<token_type>, <token_str>, [<flags>, [<str_parsed>]]])'
 *   `_END'
 */
#define EXAMPLE_TREE                                                                       \
    _BEGIN(N_DOBLK,)                                                                       \
        _BEGIN(N_ASSIGN,)                                                                  \
            _BEGIN(N_PAPPLY, T_IDENT, "map", NF_PAPPLY_REC)                                \
                _BEGIN(N_PIDENT, T_IDENT, "f",) _END                                       \
                _BEGIN(N_PIDENT, T_IDENT, "xs",) _END                                      \
            _END                                                                           \
            _BEGIN(N_MATCH,)                                                               \
                _BEGIN(N_IDENT, T_IDENT, "xs",) _END                                       \
                _BEGIN(N_CASE,)                                                            \
                    _BEGIN(N_PLIST,) _END                                                  \
                    _BEGIN(N_LIST,) _END                                                   \
                _END                                                                       \
                _BEGIN(N_CASE,)                                                            \
                    _BEGIN(N_PLIST,)                                                       \
                        _BEGIN(N_PIDENT, T_IDENT, "x",) _END                               \
                        _BEGIN(N_PLTAIL, T_IDENT, "xs", NF_PLTAIL_NAME) _END               \
                    _END                                                                   \
                    _BEGIN(N_LIST,)                                                        \
                        _BEGIN(N_APPLY,)                                                   \
                            _BEGIN(N_IDENT, T_IDENT, "f",) _END                            \
                            _BEGIN(N_IDENT, T_IDENT, "x",) _END                            \
                        _END                                                               \
                        _BEGIN(N_SPREAD,)                                                  \
                            _BEGIN(N_APPLY,)                                               \
                                _BEGIN(N_IDENT, T_IDENT, "map",) _END                      \
                                _BEGIN(N_IDENT, T_IDENT, "f",) _END                        \
                                _BEGIN(N_IDENT, T_IDENT, "xs",) _END                       \
                            _END                                                           \
                        _END                                                               \
                    _END                                                                   \
                _END                                                                       \
            _END                                                                           \
        _END                                                                               \
        _BEGIN(N_BINARY, T_OP, "$",)                                                       \
            _BEGIN(N_IDENT, T_IDENT, "print",) _END                                        \
            _BEGIN(N_APPLY,)                                                               \
                _BEGIN(N_IDENT, T_IDENT, "map",) _END                                      \
                _BEGIN(N_LAMBDA,)                                                          \
                    _BEGIN(N_PIDENT, T_IDENT, "x",) _END                                   \
                    _BEGIN(N_BINARY, T_OP, "+",)                                           \
                        _BEGIN(N_IDENT, T_IDENT, "x",) _END                                \
                        _BEGIN(N_INT, T_INT, "1",) _END                                    \
                    _END                                                                   \
                    _BEGIN(N_LIST,)                                                        \
                        _BEGIN(N_INT, T_INT, "1",) _END                                    \
                        _BEGIN(N_BINARY, T_INFIX, "`mod",)                                 \
                            _BEGIN(N_INT, T_INT, "2",) _END                                \
                            _BEGIN(N_INT, T_INT, "3",) _END                                \
                        _END                                                               \
                        _BEGIN(N_DEC, T_DEC, "3.14",) _END                                 \
                        _BEGIN(N_STRING, T_STRING, "\"string \\\"\"", 0, "string \"") _END \
                    _END                                                                   \
                _END                                                                       \
            _END                                                                           \
        _END                                                                               \
    _END

#define TESTS                       \
    _(buffer_putc)                  \
    _(buffer_printf)                \
    _(buffer_alloc)                 \
    _(buffer_read_file)             \
    _(lexer_example)                \
    _(lexer_empty)                  \
    _(lexer_unclosed_string)        \
    _(lexer_unclosed_block_comment) \
    _(lexer_unclosed_decimal)       \
    _(atoms)                        \
    _(parse_tuple_or_parens)        \
    _(elaborate_expression)         \
    _(snapshot_diff)                \
    _(snapshot_diff_empty_lines)    \
    _(snapshot_diff_line_endings)   \
    _(stack)

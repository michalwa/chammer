#define EACH_TEST(_)                    \
    _(buffer_putc)                      \
    _(buffer_printf)                    \
    _(buffer_alloc)                     \
    _(buffer_read_file)                 \
    _(lexer_full_example)               \
    _(lexer_empty)                      \
    _(lexer_empty_string_after_keyword) \
    _(lexer_unclosed_string)            \
    _(lexer_unclosed_block_comment)     \
    _(lexer_unclosed_decimal)           \
    _(lexer_under)                      \
    _(lexer_minus_and_rarrow)           \
    _(parser_full_example)              \
    _(parser_examples)                  \
    _(snapshot_diff)                    \
    _(snapshot_diff_empty_lines)        \
    _(snapshot_diff_line_endings)       \
    _(stack)                            \
    _(buffer_print_c_string_literal)

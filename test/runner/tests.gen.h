#define TESTS \
    _(buffer_printf) \
    _(buffer_putc) \
    _(buffer_alloc) \
    _(fread_to_buffer) \
    _(lexer_example) \
    _(lexer_empty) \
    _(lexer_unclosed_string) \
    _(lexer_unclosed_block_comment) \
    _(lexer_unclosed_decimal) \
    _(atoms) \
    _(parse_tuple_or_parens) \
    _(elaborate_expression) \
    _(stack)

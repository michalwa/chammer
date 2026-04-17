#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

#define PROGRAM_SOURCE "\"Hello, \" + \"world!\""

int main(void) {
    token t;
    Parser p;
    parse_result presult;
    Compiler c;
    Buffer comp_buffer;
    program prog;

    token_begin(&t, PROGRAM_SOURCE);

    parser_init(&p);
    if ((presult = parse_program(&p, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

    Buffer out;
    buffer_init(&out);
    node_print(*p.node, &out);
    printf(F_BUFFER"\n", FA_BUFFER(out));
    buffer_free(&out);

    compiler_init(&c);
    compiler_visit(&c, p.node);
    buffer_init(&comp_buffer);
    compiler_write_program(&c, &comp_buffer);
    compiler_free(&c);
    parser_free(&p);

    if (!program_read(&prog, (uint8_t *)comp_buffer.data, comp_buffer.len))
        panic("could not read compiled program");

    printf(
        "version:           %04"PRIX16"\n"
        "trace table len:   %04"PRIX16"\n"
        "string bytes len:  %08"PRIX32"\n"
        "string bytes:      %.*s\n"
        "bytecode len:      %zX\n"
        "bytecode:",
        u16be_value(*prog.version),
        u16be_value(*prog.trace_table_len),
        u32be_value(*prog.string_bytes_len),
        u32be_value(*prog.string_bytes_len),
        prog.string_bytes,
        prog.bytecode_len
    );

    for (size_t i = 0; i < prog.bytecode_len; i++) {
        printf("%s%02"PRIX8, (i % 16) ? ((i % 8) ? " " : "  ") : "\n  ", prog.bytecode[i]);
    }

    printf("\n");

    buffer_free(&comp_buffer);

    return 0;
}

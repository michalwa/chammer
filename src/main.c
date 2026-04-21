#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

#define PROGRAM_SOURCE "if foo then \"foo\" else \"bar\" + 42"

int main(void) {
    token        t;
    Parser       p;
    parse_result presult;
    Compiler     c;
    Buffer       comp_buffer;
    program      prog;

    token_begin(&t, PROGRAM_SOURCE);

    parser_init(&p);
    if ((presult = parse_program(&p, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

    Buffer out;
    buffer_init(&out);
    node_print(*p.node, &out);
    printf(F_BUFFER "\n\n", FA_BUFFER(out));
    buffer_free(&out);

    buffer_init(&comp_buffer);
    compiler_init(&c);
    compiler_visit_program(&c, p.node);
    compiler_write_program(&c, &comp_buffer);
    compiler_free(&c);
    parser_free(&p);

    if (!program_read(&prog, (uint8_t *)comp_buffer.data, comp_buffer.len))
        panic("could not read compiled program");

    printf("version:           %04" PRIX16 "\n", u16be_value(*prog.version));

    uint16_t traces_len = u16be_value(*prog.traces_len);

    printf("traces len:        %04" PRIX16 "\n", traces_len);
    printf("traces:\n");

    for (uint32_t i = 0; i < traces_len; i++) {
        uint32_t string_offset = u32be_value(prog.traces[i].string_offset);
        uint32_t string_len = u32be_value(prog.traces[i].string_len);
        printf("  %04X %.*s\n", i, string_len, prog.string_bytes + string_offset);
    }

    printf("string bytes len:  %08" PRIX32 "\n", u32be_value(*prog.string_bytes_len));
    printf("string bytes:      %.*s\n", u32be_value(*prog.string_bytes_len), prog.string_bytes);
    printf("bytecode len:      %zX\n", prog.bytecode_len);
    printf("bytecode:");

    for (size_t i = 0; i < prog.bytecode_len; i++)
        printf("%s%02" PRIX8, (i % 16) ? ((i % 8) ? " " : "  ") : "\n  ", prog.bytecode[i]);

    printf("\n");

    buffer_free(&comp_buffer);

    return 0;
}

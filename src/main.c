#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

int main(void) {
    token        t;
    Parser       p;
    parse_result presult;
    Compiler     c;
    Buffer       input;
    Buffer       comp_buffer;
    program      prog;

    FILE *example = fopen("examples/html.ham", "rb");
    buffer_init(&input);
    buffer_read_file(&input, example);
    fclose(example);

    token_begin(&t, input.data);

    parser_init(&p);
    if ((presult = parse_program(&p, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

    Buffer out;
    buffer_init(&out);
    node_print(*p.node, &out);
    printf(F_BUFFER "\n\n", FA_BUFFER(out));
    buffer_clear(&out);

    buffer_init(&comp_buffer);
    compiler_init(&c);
    compiler_visit_program(&c, p.node);
    compiler_write_program(&c, &comp_buffer);
    compiler_free(&c);
    parser_free(&p);

    if (!program_read(&prog, (uint8_t *)comp_buffer.data, comp_buffer.len))
        panic("could not read compiled program");

    printf("version:           %04" PRIX16 "\n", prog.version);
    printf("string bytes len:  %" PRIu32 "\n", prog.string_bytes_len);
    printf("string bytes:      %.*s\n", (int)prog.string_bytes_len, prog.string_bytes);
    printf("funcs len:         %" PRIu32 "\n", prog.funcs_len);
    printf("funcs:");

    for (uint32_t i = 0; i < prog.funcs_len; i++) {
        func_meta fn = program_func_meta(&prog, i);
        printf(
            "\n  %2" PRIu32 " | %08" PRIX32 " locals: %2" PRIu8 ", args: %2" PRIu8, i, fn.addr,
            fn.locals, fn.args
        );
    }

    printf("\n");
    printf("bytecode len:      %zu\n", prog.bytecode_len);
    // printf("bytecode:");

    // for (size_t i = 0; i < prog.bytecode_len; i++)
    //     printf("%s%02" PRIX8, (i % 16) ? ((i % 8) ? " " : "  ") : "\n  ", prog.bytecode[i]);

    // printf("\n\n");

    bytecode_debug_print(prog.bytecode, prog.bytecode_len, prog.string_bytes, &out);
    printf(F_BUFFER, FA_BUFFER(out));

    buffer_free(&input);
    buffer_free(&comp_buffer);
    buffer_free(&out);

    return 0;
}

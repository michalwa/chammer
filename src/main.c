#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/machine.h"
#include "../lib/parser.h"
#include "../lib/utils.h"
#include "../lib/value.h"

int main(void) {
    token        t;
    Parser       p;
    parse_result presult;
    Compiler     c;
    Buffer       input;
    Buffer       comp_buffer;
    Buffer       out;
    program      prog;
    Machine      machine;

    FILE *example = fopen("examples/html.ham", "rb");
    buffer_init(&input);
    buffer_read_file(&input, example);
    fclose(example);

    token_begin(&t, input.data);

    parser_init(&p);
    if ((presult = parse_program(&p, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

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

    program_debug_print(&prog, &out);
    printf(
        F_BUFFER "\ntotal size: %zu bytes, bytecode: %zu bytes\n", FA_BUFFER(out), comp_buffer.len,
        prog.bytecode_len
    );
    buffer_clear(&out);

    size_t steps = 0;
    machine_init(&machine, &prog);
    while (machine_step(&machine)) steps++;

    printf("\nexecuted in %zu steps\n", steps);

    machine_ctx ctx;
    machine_ctx_init(&ctx, &machine);
    for (EACH_IN_VECTOR(machine.opstack, HValue, value)) {
        buffer_puts(&out, STRING("  "));
        hvalue_print_repr(value, &out, &ctx);
        buffer_putc(&out, '\n');
    }

    printf("stack:\n" F_BUFFER "\n", FA_BUFFER(out));

    machine_free(&machine);

    buffer_free(&input);
    buffer_free(&comp_buffer);
    buffer_free(&out);

    return 0;
}

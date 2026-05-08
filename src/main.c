#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/machine.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

int main(int argc, char **argv) {
    token        t;
    Parser       p;
    parse_result presult;
    Compiler     c;
    Buffer       input;
    Buffer       comp_buffer;
    program      prog;
    Machine      machine;

#ifdef HAMMER_DEBUG
    Buffer out;
#endif

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.ham>", argv[0]);
        return 1;
    }

    FILE *example = fopen(argv[1], "rb");
    buffer_init(&input);
    buffer_read_file(&input, example);
    fclose(example);

    token_begin(&t, input.data);

    parser_init(&p);

    parser_define_operator(&p, STRING("+"), 500, ASSOC_LEFT);
    parser_define_operator(&p, STRING("-"), 500, ASSOC_LEFT);
    parser_define_operator(&p, STRING("*"), 600, ASSOC_LEFT);
    parser_define_operator(&p, STRING("/"), 600, ASSOC_LEFT);

    if ((presult = parse_program(&p, &t)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

#ifdef HAMMER_DEBUG
    buffer_init(&out);
    node_print(*p.node, &out);
    printf(F_BUFFER "\n\n", FA_BUFFER(out));
    buffer_clear(&out);
#endif

    buffer_init(&comp_buffer);
    compiler_init(&c);
    compiler_visit_program(&c, p.node);
    compiler_write_program(&c, &comp_buffer);
    compiler_free(&c);
    parser_free(&p);

    if (!program_read(&prog, (uint8_t *)comp_buffer.data, comp_buffer.len))
        panic("could not read compiled program");

#ifdef HAMMER_DEBUG
    program_debug_print(&prog, &out);
    printf(
        F_BUFFER "\ntotal size: %zu bytes, bytecode: %zu bytes\n\n", FA_BUFFER(out),
        comp_buffer.len, prog.bytecode_len
    );
    buffer_clear(&out);

    size_t steps = 0;
#endif

    machine_init(&machine, &prog);
    while (machine_step(&machine)) {
#ifdef HAMMER_DEBUG
        steps++;
#endif
    }

#ifdef HAMMER_DEBUG
    printf("\nexecuted in %zu steps\n", steps);
#endif

    machine_free(&machine);

    buffer_free(&input);
    buffer_free(&comp_buffer);
#ifdef HAMMER_DEBUG
    buffer_free(&out);
#endif

    return 0;
}

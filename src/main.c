#include "../lib/buffer.h"
#include "../lib/builtin/prelude.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/machine.h"
#include "../lib/parser.h"
#include "../lib/utils.h"
#include "../lib/value.h"

int main(int argc, char **argv) {
    token        token;
    Parser       parser;
    parse_result presult;
    Compiler     compiler;
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

    token_begin(&token, input.data);

    parser_init(&parser);

    Module prelude;
    module_init(&prelude);
    module_make_prelude(&prelude, &parser);

    if ((presult = parse_program(&parser, &token)) != PARSE_OK)
        panic("parse failed: %s", parse_result_name(presult));

#ifdef HAMMER_DEBUG
    buffer_init(&out);
    node_print(*parser.node, &out);
    printf(F_BUFFER "\n\n", FA_BUFFER(out));
    buffer_clear(&out);
#endif

    buffer_init(&comp_buffer);
    compiler_init(&compiler);
    compiler_visit_program(&compiler, parser.node);
    compiler_write_program(&compiler, &comp_buffer);
    compiler_free(&compiler);
    parser_free(&parser);

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
    machine_add_module(&machine, prelude);

    HValue error = { 0 };

    while (machine_step(&machine, &error)) {
#ifdef HAMMER_DEBUG
        steps++;
#endif
    }

    if (error.type == V_ERROR) {
        string msg = hvalue_string_get(&error);
        fprintf(stderr, "hammer error: " F_STRING "\n", FA_STRING(msg));
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

    return (error.type == V_ERROR) ? 1 : 0;
}

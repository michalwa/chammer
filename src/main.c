#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/machine.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

// clang-format off
#define PROGRAM_SOURCE "let xs = []; [...xs, ...xs, ...xs]"
// #define PROGRAM_SOURCE \
//     "let rec map f xs = match xs\n" \
//     "  case [] then []\n" \
//     "  case [x, ...rest] then [f x, ...map f xs];\n" \
//     "map (\\x -> x + 1) [1, 2, 3]\n"
// clang-format on

int main(void) {
    token        t;
    Parser       p;
    parse_result presult;
    Compiler     c;
    // Buffer       input;
    Buffer       comp_buffer;
    program      prog;

    token_begin(&t, PROGRAM_SOURCE);

    // FILE *example = fopen("examples/html.ham", "rb");
    // buffer_init(&input);
    // buffer_read_file(&input, example);
    // fclose(example);

    // token_begin(&t, input.data);

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

    printf("version:           %04" PRIX16 "\n", u16be_value(*prog.version));
    printf("string bytes len:  %" PRIu32 "\n", u32be_value(*prog.string_bytes_len));
    printf("string bytes:      %.*s\n", u32be_value(*prog.string_bytes_len), prog.string_bytes);
    printf("bytecode len:      %zu\n", prog.bytecode_len);
    printf("bytecode:");

    for (size_t i = 0; i < prog.bytecode_len; i++)
        printf("%s%02" PRIX8, (i % 16) ? ((i % 8) ? " " : "  ") : "\n  ", prog.bytecode[i]);

    printf("\n\n");

    bytecode_debug_print(prog.bytecode, prog.bytecode_len, &out);
    printf(F_BUFFER, FA_BUFFER(out));

    // Machine vm;
    // machine_init(&vm, prog.bytecode, prog.bytecode_len);

    // while (machine_step(&vm));

    // printf("\nbytecode executed\nstack:\n");

    // for (stack_iter i = stack_iter_begin(&vm.opstack); stack_iter_next(&i);) {
    //     vm_value *v = (vm_value *)i.item;
    //     switch (v->type) {
    //     case V_INT: printf("  %" PRIu64 "\n", v->value.int_value); break;
    //     default: printf("  (value)\n"); break;
    //     }
    // }

    // machine_free(&vm);

    buffer_free(&comp_buffer);
    buffer_free(&out);

    return 0;
}

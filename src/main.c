#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/compiler.h"
#include "../lib/lexer.h"
#include "../lib/machine.h"
#include "../lib/parser.h"
#include "../lib/utils.h"

#define PROGRAM_SOURCE "let f = \\x y -> \\z -> x + y + z; f 1 2 3"

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
    printf("string bytes len:  %08" PRIX32 "\n", u32be_value(*prog.string_bytes_len));
    printf("string bytes:      %.*s\n", u32be_value(*prog.string_bytes_len), prog.string_bytes);
    printf("bytecode len:      %zX\n", prog.bytecode_len);
    printf("bytecode:");

    for (size_t i = 0; i < prog.bytecode_len; i++)
        printf("%s%02" PRIX8, (i % 16) ? ((i % 8) ? " " : "  ") : "\n  ", prog.bytecode[i]);

    printf("\n");

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

    return 0;
}

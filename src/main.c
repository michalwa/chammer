#include "../lib/buffer.h"
#include "../lib/bytecode.h"
#include "../lib/utils.h"

#define PROGRAM_BYTES \
    MAGIC_HAMMER \
    "\x00\x01" /* hammer version */ \
    "\x00\x01" /* trace table length */ \
    "?" /* placeholder trace value */ \
    "\x00\x00\x00\x0D" /* string bytes length */ \
    "Hello, world!" /* string bytes */ \
    "\x01" "\x00\x00" /* trace */ \
    "\x02" "\x00\x00\x00\x00" "\x00\x00\x00\x0D" /* push string */

int main(void) {
    program prog;
    if (!program_read(&prog, (uint8_t *)PROGRAM_BYTES, sizeof(PROGRAM_BYTES) - 1))
        panic("could not read compiled program");

    printf(
        "hammer version:    %02"PRIX16"\n"
        "trace table len:   %02"PRIX16"\n"
        "trace placeholder: %c\n"
        "string bytes len:  %04"PRIX32"\n"
        "string bytes:      %.*s\n"
        "bytecode len:      %02zX\n"
        "bytecode:          ",
        u16be_value(*prog.hammer_version),
        u16be_value(*prog.trace_table_len),
        prog.trace_table[0]._placeholder,
        u32be_value(*prog.string_bytes_len),
        u32be_value(*prog.string_bytes_len),
        prog.string_bytes,
        prog.bytecode_len
    );

    for (size_t i = 0; i < prog.bytecode_len; i++) {
        printf("%02"PRIX8" ", prog.bytecode[i]);
    }

    printf("\n");

    return 0;
}

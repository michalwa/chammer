ifeq ($(origin CC), default)
	CC = clang
endif

CFLAGS += -std=c99 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wno-char-subscripts \
	-Wno-parentheses \
	-Wimplicit-fallthrough \
	-Werror=incompatible-pointer-types \
	-Werror=enum-conversion
CFLAGS_RELEASE += -O3
CFLAGS_DEBUG   += -g -O0 -DHAMMER_DEBUG -fsanitize=address -fsanitize=undefined
CFLAGS_TEST    += -g -O0 -DHAMMER_DEBUG
CFLAGS_CLANGD  += -DHAMMER_DEBUG -D_CRT_SECURE_NO_WARNINGS

ifeq ($(TEST_ASAN), 1)
	CFLAGS_TEST += -fsanitize=address -fsanitize=undefined
endif

SRC_LIB   = lib/*.c lib/*.h
SRC_BIN   = src/*.c
SRC_TEST  = test/*.c test/**/*.c test/**/*.h
SRC_BENCH = bench/*.c bench/**/*.c
SRC       = $(SRC_LIB) $(SRC_BIN) $(SRC_TEST) $(SRC_BENCH)

.PHONY: .release
release: bin/hammer

.PHONY: debug
debug: bin/hammer-debug

.PHONY: test
test: bin/test
	bin/test $(TEST)

.PHONY: bench
bench: bin/bench
	bin/bench $(if $(BENCH),--filter=$(BENCH),)

bin/hammer: $(SRC_BIN) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_RELEASE)

bin/hammer-debug: $(SRC_BIN) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_DEBUG)

bin/test: bin/build_test $(SRC_TEST) $(SRC_LIB)
	bin/build_test test/runner/tests.gen.h test/*.c
	clang-format -i test/runner/tests.gen.h
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_TEST)

bin/build_test: build_test.c test/*.c
	mkdir -p bin
	$(CC) $< -o $@ $(CFLAGS)

bin/bench: $(SRC_BENCH) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_RELEASE) -Dasm=__asm__

bin/bench-debug: $(SRC_BENCH) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_DEBUG) -Dasm=__asm__

.PHONY: format
format:
	clang-format -i $(SRC)

.PHONY: format-check
format-check:
	clang-format --dry-run -Werror $(SRC)

.clangd: Makefile
	@echo "# Run \`make .clangd\` to update" > $@
	@echo "CompileFlags:" >> $@
	@echo "  Add:" >> $@
	@for flag in -xc $(CFLAGS) $(CFLAGS_CLANGD); do \
		echo "    - $$flag" >> $@;                  \
	done
	@echo ".clangd file updated"

.PHONY: clean
clean:
	rm -rf bin

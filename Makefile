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
CFLAGS_DEBUG   += -g -O0 -fsanitize=address -fsanitize=undefined -DHAMMER_DEBUG

SRC_LIB  = lib/*.c lib/*.h
SRC_BIN  = src/*.c src/*.h
SRC_TEST = test/*.c test/**/*.c test/**/*.h
SRC      = $(SRC_LIB) $(SRC_BIN) $(SRC_TEST)

.PHONY: .release
release: bin/hammer

.PHONY: debug
debug: bin/hammer-debug

.PHONY: test
test: bin/test
	bin/test $(TEST)

bin/hammer: $(SRC_BIN) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_RELEASE)

bin/hammer-debug: $(SRC_BIN) $(SRC_LIB)
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_DEBUG)

bin/test: bin/build_test $(SRC_TEST) $(SRC_LIB)
	bin/build_test test/runner/tests.gen.h test/*.c
	$(CC) $(filter %.c, $^) -o $@ $(CFLAGS) $(CFLAGS_DEBUG)

bin/build_test: build_test.c test/*.c
	mkdir -p bin
	$(CC) $< -o $@ $(CFLAGS)

.PHONY: format
format:
	clang-format -i $(SRC)

.PHONY: format-check
format-check:
	clang-format --dry-run -Werror $(SRC)

.PHONY: clean
clean:
	rm -rf bin

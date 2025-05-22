CC             ?= clang
CFLAGS         ?= -std=c99 -Wall -Wextra -Wpedantic -Wno-char-subscripts -Wno-parentheses -Wimplicit-fallthrough
CFLAGS_RELEASE ?= -O3
CFLAGS_DEBUG   ?= -g -O0

.PHONY: .release
release: bin/hammer

.PHONY: debug
debug: bin/hammer-debug

.PHONY: test
test: bin/test
	bin/test

bin/hammer: lib/*.c lib/*.h src/*.c src/*.h
	mkdir -p bin
	$(CC) lib/*.c src/*.c -o $@ $(CFLAGS) $(CFLAGS_RELEASE)

bin/hammer-debug: lib/*.c lib/*.h src/*.c src/*.h
	mkdir -p bin
	$(CC) lib/*.c src/*.c -o $@ $(CFLAGS) $(CFLAGS_DEBUG)

bin/build_test: build_test.c test/*.c
	mkdir -p bin
	$(CC) $< -o $@ $(CFLAGS)

bin/test: bin/build_test lib/*.c lib/*.h test/*.c test/*.h
	bin/build_test test/tests.gen.h test/*.c
	$(CC) lib/*.c test/*.c -o $@ $(CFLAGS) $(CFLAGS_DEBUG)

.PHONY: format
format:
	clang-format -i **/*.{c,h}

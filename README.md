# hammer

A Haskell- and Rust-inspired functional programming language developed as a hobby project.

This is a C reimplementation of an original Rust project.

## Features

See [example.ham](test/example.ham) for a syntax example. A more complete example or specification might come later at some point.

## Development

### Folder structure

- `lib/` - Core library source code. This can potentially be compiled into a shared library.
- `src/` - Source code for the CLI frontend. For now it's just a placeholder for testing.
- `test/` - Files with the `test_` prefix define test cases. There is a custom DSL for defining tests.
  - `lib/` - Testing utilities. Includes the test definition and assertion macros.
  - `runner/` - Test entry point and index.
  - `snapshots/` - Saved snapshots for snapshot testing.
- `build_test.c` - A small C build tool for indexing tests.

### Prerequisites

- C compiler (GCC or Clang)
- GNU Make

### Building and running

Make and run the release build:

```sh
make # or `make release'
bin/hammer
```

Make and run the debug build:

```sh
make debug
bin/hammer-debug
```

Make and run tests:

```sh
make test
```

Review changed snapshots (interactive):

```sh
HAMMER_SNAPSHOT_REVIEW=1 make test
```

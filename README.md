# 🔨 hammer

an ML/Haskell/Rust-inspired interpreted functional programming language

This is a C reimplementation of an original Rust project.

## Features

See [html.ham](examples/html.ham) for a syntax example. A more complete example or specification might come later at some point.

## Development

### Folder structure

```
├─ lib           core library source code, could be compiled into a shared library in the future
├─ src           source code for the CLI frontend, for now it's just a placeholder for testing
├─ test          files with the `test_` prefix define test cases
│  ├─ lib        testing macros & utilities
│  ├─ runner     test entry point and auto-generated index
│  └─ snapshots  saved snapshots for snapshot testing
├─ build_test.c  a small build tool for indexing tests
├─ bench         files with the `bench_` prefix define benchmarks
└─ vendor        vendored third-party libraries
```

### Prerequisites

- C compiler, the project is configured for `clang` tooling
- GNU Make

### Building and running

Make and run the release build:

```sh
make # or `make release`
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
# or run a specific test
make test TEST=<test-name>
# sanitizers are disabled in tests by default to avoid cluttering output
make test TEST_ASAN=1
```

Review changed snapshots (interactive):

```sh
make test HAMMER_SNAPSHOT_REVIEW=1
```

Run benchmarks

```sh
make bench
# or run a specific benchmark
make bench BENCH=<bench-name>
```

### Code style

The project is configured to work with [clangd](https://clangd.llvm.org) and `clang-format`. Run `make format` to auto-format the entire project.

Other code style guidelines which cannot be enforced via tooling are described [here](STYLE.md). Most of these are reasonable good practice, plus some opinionated stuff.

### Third-party libraries

- [ubench.h](https://github.com/sheredom/ubench.h)

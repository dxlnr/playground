## Task

Adding a `fore` loop to clang. This is basically some non-sense functionality which helps you dive into clang a little bit.

```c
fore (int i=0; i < 10; i++) {
    printf("%d\n", i);
}
```

```bash
0
4
8
```

## Prerequisits & Run

First and formost, clang has to be compiled. Follow the instructions [here](). For getting the source code run:

```bash
cd clang && git pull
git submodule update --init --recursive
```

```bash
./clang/build/bin/clang main.c -o main
./main
```

The built `main` file you find is built with clang and runs a `fore` loop.

## Notes

- The for loop keyword `kw_for` is found in `clang/lib/Parse/ParseStmt.cpp`.
- General Definition of the language is defined in `clang/include/clang/Basic/TokenKinds.def`
- `EmitStmt(S.getInc())` within `clang/lib/CodeGen/CGStmt.cpp` actually does the job.


## Additionals

- Clang: [Internal Manual](https://clang.llvm.org/docs/InternalsManual.html)

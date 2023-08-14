## Task

Adding `fore` loop to clang.

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
clang 
```

## Notes

The for loop keyword `kw_for` is found in `clang/lib/Parse/ParseStmt.cpp`.
General Definition of the language is defined in `clang/include/clang/Basic/TokenKinds.def`

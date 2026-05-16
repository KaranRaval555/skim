# skim

- A simple educational tree-walking interpreter implementing a subset of Scheme in C.

## Run REPL

```bash
gcc skim.c -o skim
./skim
```

## Run Tests

```bash
gcc -DSKIM_TEST skim.c tests.c -o tests
./tests
```

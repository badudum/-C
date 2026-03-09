# -C

A compiler for the **minusC** language targeting ARM64 assembly on macOS.

## Build

```bash
make
```

## Run

```bash
./minusC.out example/main.minusc
as -arch arm64 mc.s -o mc.o
ld -e _start -macos_version_min 11.0.0 -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lSystem -o mc_test mc.o
./mc_test
```

## Embedded Features

### Built-in Functions

| Function | Description |
|----------|-------------|
| `HelloWorld(arg1, arg2, ...)` | Prints each argument to stdout. Accepts strings and integers (ints are converted to strings automatically). |
| `HelloWorldLine(arg1, arg2, ...)` | Same as `HelloWorld` but appends a newline after printing all arguments. |

### Data Types

- **int** — 32-bit integers
- **str** — String literals and variables
- **bool** — Boolean type (parser support)

### Variables

```minusC
{name} str = "Hello";
{x} int = 42;
{mixed} int = (2 + 3) * 4;
```

### Functions

```minusC
print = ({test} str) function {
    HelloWorld(test);
    return 30;
} int;

main = ({x} int) function {
    return 0;
} int;
```

### Arithmetic

- Operators: `+`, `-`, `*`, `/`
- Order of operations: multiplication and division before addition and subtraction
- Parentheses for grouping: `(2 + 3) * 4`

### Strings

- Double-quoted literals: `"Hello, world!"`
- Escape sequences: `\n` (newline), `\t` (tab), `\r` (carriage return), `\\`, `\"`, `\'`

### Comments

- Line comment: `comment` followed by rest of line
- Block comment: `comment:` until `;`

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

## Language Features

### Data Types

| Type | Description |
|------|-------------|
| `int` | 32-bit integers |
| `str` | String literals and variables |
| `bool` | Boolean — `Real` (true) or `Fake` (false) |
| `Array<int>` | Integer arrays |

### Variables

Variables are declared with the `{name} type = value;` syntax.

```minusC
{x} int = 42;
{name} str = "Hello";
{arr} Array<int> = [10, 20, 30];
```

### Booleans

Boolean values use `Real` (true) and `Fake` (false). When printed, they display as `Real` or `Fake`. When used arithmetically, they behave as `1` or `0`.

```minusC
{t} bool = Real;
{f} bool = Fake;
HelloWorldLine("value: ", t);   // prints "value: Real"
```

### Arithmetic

- Operators: `+`, `-`, `*`, `/`
- Operator precedence: `*` and `/` bind tighter than `+` and `-`
- Parentheses for grouping: `(2 + 3) * 4`

```minusC
{mixed} int = 2 + 3 * 4;       // 14
{grouped} int = (2 + 3) * 4;   // 20
{nested} int = ((1 + 2) * 3) + 4;
```

### Comparison Operators

All comparisons return a `bool` (`Real` or `Fake`).

| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |
| `<` | Less than |
| `>` | Greater than |
| `<=` | Less than or equal |
| `>=` | Greater than or equal |

```minusC
{eq} bool = 5 == 5;   // Real
{lt} bool = 3 < 5;    // Real
{ge} bool = 3 >= 5;   // Fake
```

### Logical Operators

| Operator | Description |
|----------|-------------|
| `and` | Logical AND — returns `Real` if both operands are truthy |
| `or` | Logical OR — returns `Real` if either operand is truthy |
| `not` | Logical NOT — negates a boolean value |

```minusC
{a} bool = Real and Fake;   // Fake
{o} bool = Real or Fake;    // Real
{n} bool = not Fake;         // Real
```

### Bitwise Operators

| Operator | Description |
|----------|-------------|
| `&` | Bitwise AND |
| `\|` | Bitwise OR |
| `~` | Bitwise NOT |

```minusC
{ba} int = 5 & 3;   // 1
{bo} int = 5 | 3;   // 7
{bn} int = ~0;      // -1 (all bits set)
```

### If / Else If / Else

Conditional branching with `if`, `else if`, and `else`. The condition must be in parentheses and the body in braces. The entire if statement is terminated with a semicolon after the closing brace.

```minusC
{n} int = 10;
if (n > 20) {
    HelloWorldLine("big");
} else if (n > 5) {
    HelloWorldLine("medium");
} else {
    HelloWorldLine("small");
};
```

Standalone `if`:

```minusC
if (val == 42) {
    HelloWorldLine("found it");
};
```

Boolean literals work directly as conditions:

```minusC
if (Real) {
    HelloWorldLine("always runs");
};
```

### Loop until

A single construct that behaves as **while**, **for**, or **do-while** depending on syntax:

1. **While** — condition first, then body: `loop until (condition) { body };`
2. **For** — init; condition; step: `loop until (init; condition; step) { body };`
3. **Do-while** — body first, then condition: `loop { body } until (condition);` or with for-style: `loop { body } until (init; condition; step);`

Terminate the whole statement with a semicolon after the closing brace.

**While-style:**

```minusC
{count} int = 0;
loop until (count < 3) {
    HelloWorldLine("count=", count);
    count++;
};
```

**For-style** (init can be assignment, e.g. `i = 0`):

```minusC
{i} int = 0;
loop until (i = 0; i < 10; i++) {
    HelloWorldLine("i=", i);
};
```

**Do-while-style** (body runs at least once):

```minusC
{n} int = 0;
loop {
    HelloWorldLine("body n=", n);
    n++;
} until (n >= 2);
```

### Increment and decrement (++ / --)

Prefix and postfix are supported: `++i`, `i++`, `--i`, `i--`. Postfix returns the value before the update; prefix returns the value after the update.

```minusC
{a} int = 5;
{b} int = a++;   // b = 5, a = 6
{c} int = ++a;   // c = 7, a = 7
{d} int = 10;
{e} int = d--;   // e = 10, d = 9
{f} int = --d;   // f = 8, d = 8
```

### Strings

- Double-quoted literals: `"Hello, world!"`
- Escape sequences: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`
- Concatenation with `+`:

```minusC
{a} str = "Hello";
{b} str = "World";
{c} str = a + b;    // "HelloWorld"
```

### String Operations

| Syntax / Function | Description |
|---|---|
| `s[i]` | Character access — returns the character at index `i` |
| `s[start:end]` | Substring slice — returns characters from `start` to `end` (exclusive) |
| `SmolString(s, start, end)` | Function form of substring slice |
| `Change(s, old, new)` | Replace first occurrence of `old` with `new` in `s` |
| `Clipper(s)` | Trim leading and trailing whitespace |
| `SmolStrings(s, delim)` | Split string `s` by delimiter `delim` |

```minusC
{s} str = "hello";
{ch} str = s[0];                    // "h"
{sub} str = s[1:4];                 // "ell"
{r} str = Change("foo bar", "bar", "baz");  // "foo baz"
{trimmed} str = Clipper("  hi  ");  // "hi"
{parts} str = SmolStrings("a,b,c", ",");
```

### Arrays

Arrays are declared with the `Array<int>` type. Elements are accessed by index with `arr[i]`.

**Explicit literals:**

```minusC
{arr} Array<int> = [10, 20, 30];
{val} int = arr[0];    // 10
```

**Repeat fill** — `[value; count]` creates `count` elements all set to `value`:

```minusC
{zeros} Array<int> = [0; 5];      // [0, 0, 0, 0, 0]
{sevens} Array<int> = [7; 3];     // [7, 7, 7]
```

**Range fill** — `[v1; c1, v2; c2, ...]` concatenates multiple repeat segments:

```minusC
{range} Array<int> = [0; 3, 1; 2, 99; 1];
// produces [0, 0, 0, 1, 1, 99]
```

### Functions

Functions are declared with parameters, a body, and a return type.

```minusC
add = ({a} int, {b} int) function {
    return a + b;
} int;

greet = ({name} str) function {
    HelloWorldLine("Hello, ", name);
    return 0;
} int;
```

### Built-in I/O

| Function | Description |
|----------|-------------|
| `HelloWorld(arg1, arg2, ...)` | Prints each argument to stdout. Accepts strings and integers (ints are converted automatically). |
| `HelloWorldLine(arg1, arg2, ...)` | Same as `HelloWorld` but appends a newline. |

### Comments

```minusC
comment this is a line comment

comment:
    this is a block comment
    spanning multiple lines
;
```

## Error Handling

### Compile-Time Errors

| Error | Description |
|-------|-------------|
| Undefined variable | Using a variable that hasn't been declared |
| Type mismatch | Assigning a value to a variable of an incompatible type (e.g. array to int, str to int) |

### Runtime Errors

| Error | Description |
|-------|-------------|
| Array out-of-bounds | Accessing an array index outside `[0, size)` |
| Null string access | Indexing or slicing a null/uninitialized string |
| Division by zero | Dividing an integer by zero |

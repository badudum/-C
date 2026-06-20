# -C

A compiler for the **minusC** language targeting **ARM64 assembly on macOS**.

The compiler preprocesses source files, builds an AST, runs type and borrow checks, emits ARM64 assembly with an embedded runtime, and links a native binary. See `docs/ASSEMBLY_CONTROL_FLOW.md` for how control flow and the stack frame are lowered to assembly.

## Build

```bash
make
```

## Run

The compiler writes `mc.s`, assembles, and links automatically:

```bash
./minusC.out example/main.minusc
./mc.out
```

Manual assemble/link (equivalent):

```bash
./minusC.out example/main.minusc
as -arch arm64 mc.s -o mc.o
ld -e _start -macos_version_min 11.0.0 -L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib -lSystem -lpthread -o mc_test mc.o
./mc_test
```

## Compiler pipeline

1. **Preprocessor** — resolves `reference` includes and merges source files
2. **Lexer** — tokenizes source
3. **Parser** — builds the AST
4. **Visitor** — assigns stack slots, type checks, borrow checking for `adr`
5. **Assembler** — emits ARM64 + bootstrap runtime (I/O, strings, heap, threads)
6. **Link** — `as` + `ld` with `-lSystem` and `-lpthread`

## Language features

### Data types

| Type | Description |
|------|-------------|
| `int` | 32-bit integers |
| `str` | String literals and variables; single-character access returns `str` |
| `bool` | Boolean — `Real` (true) or `Fake` (false) |
| `Array<int>` | Integer arrays on the stack |
| `adr` | Heap pointer — owns a `rent` allocation until `moveOut` |
| Custom (`cust`) | User-defined struct types (see below) |

### Variables

Variables are declared with the `{name} type = value;` syntax.

```minusC
{x} int = 42;
{name} str = "Hello";
{arr} Array<int> = [10, 20, 30];
{buf} adr = rent(64);
```

### File includes

Include other source files with `reference`. Paths are resolved relative to the current file; a `.minusc` suffix is added if missing. Duplicate includes are skipped.

```minusC
reference array_tests
reference "example/lib/helper.minusc"
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
- `%` (modulus) is not yet implemented

```minusC
{mixed} int = 2 + 3 * 4;       // 14
{grouped} int = (2 + 3) * 4;   // 20
{nested} int = ((1 + 2) * 3) + 4;
```

### Comparison operators

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

### Logical operators

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

### Bitwise operators

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

### If / else if / else

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

**For-style** — init can be a **declaration in the bracket** or an assignment:

```minusC
loop until ({i} int = 0; i < 10; i++) {
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

### Compound assignment (`+=` and `-=`)

`x += expr` adds the value of `expr` to `x`; `x -= expr` subtracts. The variable must already be declared.

```minusC
{x} int = 100;
x += 10;   // x is now 110
x -= 25;   // x is now 85
```

### Increment and decrement (`++` / `--`)

Prefix and postfix are supported: `++i`, `i++`, `--i`, `i--`. Postfix returns the value before the update; prefix returns the value after the update.

```minusC
{a} int = 5;
{b} int = a++;   // b = 5, a = 6
{c} int = ++a;   // c = 7, a = 7
```

### Strings

- Double-quoted literals: `"Hello, world!"`
- Escape sequences: `\n`, `\t`, `\r`, `\\`, `\"`, `\'`
- Concatenation with `+`

```minusC
{a} str = "Hello";
{b} str = "World";
{c} str = a + b;    // "HelloWorld"
```

### String operations

| Syntax / function | Description |
|---|---|
| `s[i]` | Character access — returns the character at index `i` |
| `s[start:end]` | Substring slice — characters from `start` to `end` (exclusive) |
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

### Custom types (`cust`)

Define struct-like types with the `cust` keyword. Fields use the usual `{name} type;` syntax. Nested custom types are supported.

```minusC
Point = cust {
    {x} int;
    {y} int;
};

Rect = cust {
    {origin} Point;
    {w} int;
    {h} int;
};

{p} Point = Point{x=3, y=4};           // typed initializer
{q} Point = {x=10, y=5};               // bare brace initializer
{sum} int = p.x + p.y;
p.x = 20;
{r} Rect = Rect{origin=Point{x=1, y=2}, w=30, h=40};
{b} Point = a;                         // copy assign
{sz} int = sizeof(Point);              // 8
```

### `sizeof`

Returns the byte size of a type at compile time. Works for primitives, `Array<int>`, and custom types.

```minusC
{si} int = sizeof(int);       // 4
{sp} int = sizeof(Point);     // field sizes summed
```

### Functions

Functions are declared with parameters, a body, and a return type. They may return any supported type, including `str` and `adr`.

```minusC
add = ({a} int, {b} int) function {
    return a + b;
} int;

makeBlock = ({val} int) function {
    {p} adr = rent(16);
    PokeInt(p, 0, val);
    return p;
} adr;
```

### Built-in I/O

| Function | Description |
|----------|-------------|
| `HelloWorld(arg1, arg2, ...)` | Prints each argument to stdout. Accepts strings and integers. |
| `HelloWorldLine(arg1, arg2, ...)` | Same as `HelloWorld` but appends a newline. |

### Heap memory (`adr`)

Heap blocks are allocated with `rent` and freed with `moveOut`. Each `adr` variable owns its allocation; assigning one `adr` to another **moves** ownership (the source becomes invalid). The compiler enforces this at compile time (see borrow checking below).

| Function / syntax | Description |
|---|---|
| `rent(bytes)` | Allocate `bytes` zero-initialized bytes on the heap |
| `rent(count, type)` | Allocate `count * sizeof(type)` bytes (e.g. `rent(5, int)`) |
| `moveOut(adr)` | Free the heap block; returns `0` on success |
| `PeekInt(adr, offset)` | Read a 32-bit int at byte offset |
| `PokeInt(adr, offset, val)` | Write a 32-bit int at byte offset |
| `PeekByte(adr, offset)` | Read one byte at offset |
| `PokeByte(adr, offset, val)` | Write one byte at offset |
| `AddInt(adr, offset, delta)` | Atomically add `delta` to int at offset (thread-safe) |
| `AdrLo(adr)` / `AdrHi(adr)` | Low/high parts of the heap address |
| `Memcpy(dst, src, len)` | Copy `len` bytes between heap blocks |
| `Memset(adr, byte, len)` | Fill `len` bytes with `byte` |
| `RentGrow(adr, new_size)` | Grow block, copy old data, free old block |
| `adr[offset]` | Byte index access on a heap block |

```minusC
{buf} adr = rent(64);
PokeInt(buf, 0, 42);
{v} int = PeekInt(buf, 0);
{b} int = buf[20];          // byte at offset 20
{fd} int = moveOut(buf);     // buf is now invalid

{arr} adr = rent(5, int);
PokeInt(arr, 0, 10);
PokeInt(arr, 4, 20);
```

### Borrow checking (`adr`)

The compiler tracks `adr` ownership per function:

- After `moveOut(x)`, using `x` is a **compile error**
- Assigning `adr` to another `adr` moves ownership (source is invalidated)
- Returning a moved `adr`, passing a moved `adr` to `dupe`, or using moved `adr` in heap builtins is rejected at compile time

See `example/borrow_tests.minusc` (valid) and `example/borrow_fail.minusc` (intentional failure).

### Threading

Threads are spawned with the `dupe` keyword (not a function call). The runtime uses pthreads.

| Function / syntax | Description |
|---|---|
| `dupe(func, arg)` | Spawn a thread that calls `func` with `arg`; returns a thread slot index |
| `join(slot)` | Wait for the thread; returns `0` on success |
| `timer()` | Microsecond timestamp for benchmarking |

`dupe` accepts an `int` or `adr` argument. For `adr`, the compiler checks that the pointer has not been moved.

```minusC
worker = ({n} int) function {
    return n * 2;
} int;

{slot} int = dupe(worker, 7);
{result} int = join(slot);

{t0} int = timer();
{acc} int = busyWork(5000);
{t1} int = timer();
HelloWorldLine("elapsed_us=", t1 - t0);
```

Use `AddInt` instead of `PeekInt` + `PokeInt` for shared counters across threads.

### Comments

```minusC
comment this is a line comment

comment:
    this is a block comment
    spanning multiple lines
;
```

## Error handling

### Compile-time errors

| Error | Description |
|-------|-------------|
| Undefined variable | Using a variable that has not been declared |
| Type mismatch | Incompatible assignment (e.g. `str` to `int`, mismatched `cust` types) |
| Borrow error | Use of moved `adr`, double `moveOut`, invalid `adr` in call/return |
| Unknown field | Invalid field name in `cust` initializer or field access |
| Invalid `sizeof` | Unsupported or zero-size type |

### Runtime errors

| Error | Description |
|-------|-------------|
| Array out-of-bounds | Array index outside `[0, size)` |
| Null string access | Indexing or slicing a null/uninitialized string |
| Division by zero | Integer division by zero |
| Heap out of memory | `rent` could not allocate |
| Invalid heap size | `rent` with zero or negative size |
| Invalid heap address | `moveOut` or heap access on invalid pointer |
| Double free | `moveOut` called twice on the same block |
| Heap access out-of-bounds | `Peek`/`Poke`/`adr[offset]` past block end |

## Examples

| File | Coverage |
|------|----------|
| `example/main.minusc` | Full integration test driver |
| `example/array_tests.minusc` | Arrays and literals |
| `example/bool_if_tests.minusc` | Booleans, comparisons, if/else |
| `example/loop_tests.minusc` | `loop until` variants |
| `example/string_ops.minusc` | String builtins |
| `example/heap_tests.minusc` | Heap allocator and peek/poke |
| `example/borrow_tests.minusc` | Valid borrow/ownership patterns |
| `example/mem_tests.minusc` | `sizeof`, `Memcpy`, `Memset`, `RentGrow` |
| `example/thread_tests.minusc` | `dupe`, `join`, `timer`, parallel races |
| `example/cust_tests.minusc` | Custom types |

## Editor support

The `vscode-minusc/` extension provides syntax highlighting and a document formatter for `.minusc` files in VS Code and Cursor. See `vscode-minusc/README.md` for install instructions.

## Platform notes

- **Target:** ARM64 macOS (`arm64-apple-darwin`)
- **Requirements:** Xcode Command Line Tools (`as`, `ld`, macOS SDK)
- Cross-compilation to other platforms is not yet supported; the emitted assembly and bootstrap runtime are macOS-specific.

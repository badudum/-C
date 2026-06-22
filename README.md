# -C

A compiler for the **minusC** language targeting **native macOS binaries** (ARM64 and x86_64).

The compiler preprocesses source files, builds an AST, runs type and borrow checks, emits assembly with an embedded runtime, and links a native binary. See `docs/ASSEMBLY_CONTROL_FLOW.md` for how control flow and the stack frame are lowered to assembly.

## Build

```bash
make
```

This produces a universal `minusC.out` (ARM64 + x86_64). Pass a codegen target when compiling programs:

```bash
./minusC.out --arm64 example/main.minusc    # default on Apple Silicon
./minusC.out --x86_64 example/main.minusc   # Intel macOS
./minusC.out --target arm64 example/main.minusc
```

Supported targets today: **macOS ARM64** and **macOS x86_64** (Linux x86_64 codegen exists but is not fully documented/tested in CI).

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
| `int` | 32-bit integers (default integer type) |
| `float` | 64-bit floating point (default float type; not C’s 32-bit `float`) |
| `double int` | 64-bit integer — prefix `double` doubles width (see below) |
| `double float` | 128-bit floating point |
| `str` | String literals and variables; single-character access returns a one-character `str` (there is no separate `char` type — use `str` for character data) |
| `bool` | Boolean — `Real` (true) or `Fake` (false) |
| `Array<int>` | Integer arrays on the stack |
| `adr` | Heap pointer — owns a `rent` allocation until `moveOut` |
| Custom (`cust`) | User-defined struct types (see below) |
| Generic `cust` | Parameterized types monomorphized at use sites, e.g. `Box<int>` |
| Interface | Named method contract; types declare `implements` (see below) |

### Extended numeric types

Prefix a numeric type with `double` to double its bit width. Each additional `double` doubles again:

| Declaration | Width |
|-------------|-------|
| `int` | 32 bits |
| `double int` | 64 bits |
| `double double int` | 128 bits |
| `double double double int` | 256 bits |
| `double double double double int` | 512 bits |
| `float` | 64 bits |
| `double float` | 128 bits |
| … | up to 1024 bits (`double` × 4 on `float`) |

Literals: integer literals (`42`, `10000000000`) and float literals (`3.5`, `.5`, `1e10`).

Binary operations use Java-like promotion: if either operand is floating point, the result is floating point (at least 64-bit); otherwise the wider integer type wins. Mixed `int` + `float` promotes the integer to float.

```minusC
{a} double int = 10000000000;
{b} double int = 3;
{c} double int = a + b;

{f} float = 3.5;
{g} float = 2.0;
{h} float = f * g;

{i} int = 10;
{j} float = 2.5;
{k} float = i + j;   // int promoted to float → 12.5

{w} int = sizeof(int);          // 4
{w64} int = sizeof(double int); // 8
{wf} int = sizeof(float);       // 8
{wf128} int = sizeof(double float); // 16
```

Run `make test-numeric` for the runtime suite (`example/numeric_tests.minusc`), including massive values (10^19 integers, 128-bit carry, 10^18 floats).

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

Use **`reference qualified <path>`** when you want included top-level symbols namespaced under the module name (the basename of the file, without `.minusc`). Call exported functions as `module_name.symbol(...)`. Unqualified calls to module-local symbols are rejected at compile time.

```minusC
reference array_tests
reference "example/lib/helper.minusc"
reference qualified module_helper

main = () function {
    {v} int = module_helper.moduleAnswer(0);
    return 0;
} int;
```

### Standard library (`std.*`)

Include bundled library modules with `reference std.<name>` (resolved from the compiler’s `std/` directory, or `MINUSC_STD` if set):

| Module | Contents |
|--------|----------|
| `std.result` | `ResultInt` cust type, `OkInt` / `ErrInt` helpers |
| `std.math` | `AbsInt`, `MaxInt`, `MinInt` |
| `std.io` | `ReadAllText`, `WriteAllText` wrappers |
| `std.alloc` | `newArena`, `newPool` helpers |

```minusC
reference std.result

try {r} ResultInt = ResultInt{ok=Fake, code=42, val=0} else {
    HelloWorldLine("error code", r.code);
    return 1;
};
HelloWorldLine("ok value", r.val);
```

### Alternative allocators

In addition to heap `rent` / `moveOut`, the runtime provides bump **arenas** and fixed-size **pools** (compatible with `PeekInt` / `PokeInt`; arena/pool blocks skip `_free` on `moveOut`):

| Builtin | Role |
|---------|------|
| `ArenaCreate(bytes)` | Create arena; returns `adr` handle |
| `arenaRent(arena, nbytes)` | Bump-allocate within arena |
| `ArenaReset(arena)` | Reset bump pointer |
| `ArenaDestroy(arena)` | Free entire arena |
| `PoolCreate(blockSize, count)` | Create fixed-block pool |
| `poolRent(pool)` | Rent one block |
| `PoolReset(pool)` | Return all blocks to free list |
| `PoolDestroy(pool)` | Free pool memory |

### Structured errors (`try` / `else`)

Use a `ResultInt`-style cust (see `std.result`) with a bool `ok` field, then **`try {var} Type = expr else { ... }`**. If `var.ok == Fake`, the `else` block runs; otherwise execution continues after the `try` statement.

Compile-time helpers: `sizeof(T)` and integer expressions with enum constants are folded where possible (const array indices, etc.).

### Booleans

Boolean values use `Real` (true) and `Fake` (false). When printed, they display as `Real` or `Fake`. When used arithmetically, they behave as `1` or `0`.

```minusC
{t} bool = Real;
{f} bool = Fake;
HelloWorldLine("value: ", t);   // prints "value: Real"
```

### Arithmetic

- Operators: `+`, `-`, `*`, `/`, `^` (exponentiation)
- `^` binds tighter than `*`/`/` and is **right-associative** (`2^3^2` = `2^(3^2)` = 512)
- Integer `^` uses integer exponentiation (non-negative exponent; negative exponent yields 0)
- If either operand is floating point, `^` uses floating-point power (`pow`) with Java-like promotion
- Operator precedence: `*` and `/` bind tighter than `+` and `-`
- Parentheses for grouping: `(2 + 3) * 4`
- `%` (modulus) works on `int`, `double int`, and wider integer types

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

Arrays use `Array<T>` where `T` is `int`, `float`, `str`, or a stack `cust` type. Elements are accessed by index with `arr[i]`. Repeat-fill syntax `[value; count]` is supported for integer literals only.

**Explicit literals:**

```minusC
{arr} Array<int> = [10, 20, 30];
{fs} Array<float> = [1.0, 2.5, 3.5];
{words} Array<str> = ["a", "b"];
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

**Length** — `arr.len` is a compile-time constant when the array size is known from its initializer:

```minusC
{nums} Array<int> = [1, 2, 3];
{n} int = nums.len;   // 3
```

**Foreach** — iterate index `0 .. len-1`:

```minusC
foreach {i} int in nums {
    HelloWorldLine(nums[i]);
};
```

### Enums, inference, and immportal

**Enums** — named integer constants (auto-numbered from 0, or explicit values):

```minusC
enum Color { Red, Green, Blue };
enum Step { Wait = 10, Go, Done };
```

**Type inference** — omit the type on first binding when the RHS is unambiguous:

```minusC
{x} = 42;
{words} = "hello";
{bits} = [1, 2, 3];
```

**immportal** — bindings or cust fields that cannot be reassigned:

```minusC
{limit} immportal int = 100;

Config = cust {
    {version} immportal int;
    {name} str;
};
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

The `class` keyword is an alias for `cust`.

#### Methods, visibility, and `self`

Methods are defined inside the type body. Instance methods take `(self)` as the first parameter; the compiler desugars `obj.method(args)` to a mangled call with the receiver as the first argument.

```minusC
Point = cust {
    public {x} int;
    private {y} int;

    lenSq = (self) function {
        return self.x * self.x + self.y * self.y;
    } int;
};

{p} Point = Point{x=3, y=4};
{n} int = p.lenSq();
```

Use `public` / `private` on fields and methods. Cross-type access to `private` members is rejected at compile time.

#### Inheritance and virtual dispatch

Single inheritance is supported with `extends`. Mark overriding methods with `virtual` for runtime dispatch through a vtable (required for heap objects passed to helpers that only know the base type).

```minusC
Animal = cust {
    virtual speak = (self) function { return 1; } int;
};

Dog = cust extends Animal {
    virtual speak = (self) function { return 2; } int;
};

speakThrough = ({p} adr) function {
    return p.speak();   // virtual dispatch from heap handle
} int;
```

Use `super.method()` inside a derived method to call the base implementation. See `example/poly_tests.minusc` and `make test-poly`.

#### Heap objects (`rent` + methods)

Heap-backed objects use `rent(1, MyType)` and treat `self` as an `adr` inside methods:

```minusC
Counter = cust {
    {n} int;
    init = (self) function { self.n = 0; return 0; } int;
    inc = (self) function { self.n = self.n + 1; return self.n; } int;
};

{c} adr = rent(1, Counter);
c.init();
c.inc();
c.drop();
moveOut(c);
```

See `example/heap_oop_tests.minusc` and `make test-heap-oop`.

### Generic types

Generic `cust` / `class` types take type parameters in angle brackets. The compiler **monomorphizes** each use into a concrete type with a mangled name (for example `Box<int>` becomes `Box_int` internally).

**Define a template:**

```minusC
Box<T> = cust {
    {value} T;
};
```

**Instantiate at use sites** with concrete type arguments:

```minusC
{b} Box<int> = {value=42};
{p} Box<Point> = {value=Point{x=1, y=2}};
```

Rules and limits (v1):

| Topic | Behavior |
|-------|----------|
| Type arguments | Any non-generic field type: `int`, `bool`, `str`, `adr`, nested `cust`, `Array<T>`, other monomorphized generics |
| Monomorphization | First use of `Box<int>` registers `Box_int`; later uses share the same layout |
| `extends` / `implements` | `implements` allowed on generic templates (checked after monomorphization); `extends` on templates still rejected |
| Generic methods | Supported on generic `cust` types |
| `sizeof` | Works on instantiated types: `sizeof(Box<int>)` |

```minusC
{si} int = sizeof(Box<int>);
{sp} int = sizeof(Box<Point>);
```

```minusC
TaggedBox<T> = cust implements Showable {
    {value} T;
    virtual readTag = (self) function { return 1; } int;
};
```

Each instantiation (`TaggedBox<int>`, etc.) is checked against the interface when first used.

See `example/generic_tests.minusc` and `make test-generic`.

### Interfaces

An **interface** declares method signatures that concrete types must implement. Satisfaction is **nominal**: the type must list the interface with `implements`, and the compiler checks that every interface method exists with a matching return type.

**Define an interface** (method bodies are stubs — they are not emitted):

```minusC
Drawable = interface {
    draw = (self) function {
        return 0;
    } int;
};
```

**Implement on a type.** Interface methods must be `virtual` and occupy the vtable slot assigned by the interface (first method → slot 0, second → slot 1, …):

```minusC
Circle = cust implements Drawable {
    {radius} int;

    virtual draw = (self) function {
        return self.radius;
    } int;
};
```

**Interface-typed parameters** accept heap handles (`adr`) or **stack `cust` values** whose type implements the interface. Stack values are copied into a temporary vtable-prefixed buffer at the call site; heap handles are passed as-is. The parameter erases to `adr` at codegen; calls to interface methods use virtual dispatch.

```minusC
drawIt = ({shape} Drawable) function {
    return shape.draw();
} int;

{c} adr = rent(1, Circle);
c.init();
{v} int = drawIt(c);   // heap handle

{sq} Square = {side=11};
{v} int = scaleIt(sq); // stack cust — also OK when Square implements Scalable
```

Requirements:

- Implementors are checked at type definition time (`implements Drawable`), including monomorphized generic types.
- Call sites are checked when passing a heap handle or stack `cust` to an interface-typed parameter.
- Virtual methods on the implementor must match the interface’s vtable slot index.
- Stack `cust` types with `virtual` methods use heap-style field offsets inside methods; prefer heap `rent` for types that mix stack use and virtual methods on the same receiver layout.

See `example/interface_tests.minusc` and `make test-interface`.

### `sizeof`

Returns the byte size of a type at compile time. Works for primitives, `Array<int>`, custom types, and monomorphized generics (`sizeof(Box<int>)`).

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
| `ReadLine()` | Read one line from stdin (max 8 KiB; strips control chars). Returns `str`. |
| `ReadChar()` | Read one byte from stdin. Returns `-1` on EOF. |
| `KeyAvailable()` | Non-blocking check: `1` if stdin has input, else `0`. |
| `PollKey(timeout_ms)` | Wait up to `timeout_ms` (capped at 60s) for a key; returns byte or `-1`. |

Stdin is typically **line-buffered** in cooked terminal mode: keys may not appear until Enter unless the terminal is in raw mode. `PollKey`/`KeyAvailable` poll the underlying fd; they do not install signal handlers.
| `FileOpen(path, mode)` | Open a file. Mode is exactly `"r"`, `"w"`, or `"a"`. Returns fd or `-1`. |
| `FileRead(fd)` | Read entire file from current offset (max 8 MiB). Returns `str`. Does not close fd. |
| `FileWrite(fd, data)` | Write string to open fd (max 8 MiB). Returns bytes written or `-1`. |
| `FileClose(fd)` | Close fd. Returns `0` or `-1`. |
| `WriteFile(path, data)` | Create/truncate and write file in one call. Returns bytes or `-1`. |

**I/O security:** File paths must be relative (no leading `/` unless `MC_IO_ALLOW_ABSOLUTE=1`), must not contain `..` or `~`, and are limited to 1024 characters. Sensitive system paths (`/etc`, `/proc`, `/dev`, etc.) are always rejected. Files open with `O_NOFOLLOW` where supported; new files are created mode `0600`. Reads and writes are capped at 8 MiB. This is **not a sandbox** — compiled programs run with your user privileges; treat untrusted minusC code like any native binary.

### Heap memory (`adr`)

Heap blocks are allocated with `rent` and freed with `moveOut`. Each `adr` variable owns its allocation; assigning one `adr` to another **moves** ownership (the source becomes invalid). The compiler enforces this at compile time (see borrow checking below).

| Function / syntax | Description |
|---|---|
| `rent(bytes)` | Allocate `bytes` zero-initialized bytes on the heap |
| `rent(count, type)` | Allocate `count * sizeof(type)` bytes (e.g. `rent(5, int)`) |
| `moveOut(adr)` | Free the heap block; returns `0` on success |
| `PeekInt(adr, offset)` | Read a 32-bit int at byte offset |
| `PokeInt(adr, offset, val)` | Write a 32-bit int at byte offset |
| `PeekI64(adr, offset)` | Read a 64-bit integer at byte offset |
| `PokeI64(adr, offset, val)` | Write a 64-bit integer at byte offset |
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
| Unknown type | Type name not found (including uninstantiated generic use) |
| Generic arity | Wrong number of type arguments for a generic template |
| Interface mismatch | Type missing `implements` method, wrong return type, or non-virtual / wrong vtable slot |
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

## Testing

Focused test suites (after `make`):

| Command | Coverage |
|---------|----------|
| `make test-borrow` | `adr` ownership and borrow checking |
| `make test-cust` | Custom types |
| `make test-oop` | Methods, `self`, visibility |
| `make test-heap-oop` | Heap objects + methods |
| `make test-poly` | Inheritance + virtual dispatch |
| `make test-generic` | Generic type monomorphization |
| `make test-interface` | Interfaces + `implements` |
| `make test-module` | Qualified module namespacing |
| `make test-numeric` | Extended int/float types, promotion, `sizeof` |
| `make test-all` | All of the above |

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
| `example/oop_tests.minusc` | Methods and encapsulation |
| `example/heap_oop_tests.minusc` | Heap-backed objects |
| `example/poly_tests.minusc` | Virtual dispatch and inheritance |
| `example/generic_tests.minusc` | Generic `Box<T>` monomorphization |
| `example/interface_tests.minusc` | Interfaces and `implements` |

Implementation progress and design notes: `docs/OOP_CHECKLIST.md`.

## Editor support

The `vscode-minusc/` extension provides syntax highlighting and a document formatter for `.minusc` files in VS Code and Cursor. See `vscode-minusc/README.md` for install instructions.

## Platform notes

- **Target:** ARM64 macOS (`arm64-apple-darwin`)
- **Requirements:** Xcode Command Line Tools (`as`, `ld`, macOS SDK)
- Cross-compilation to other platforms is not yet supported; the emitted assembly and bootstrap runtime are macOS-specific.

# minusC Feature Checklist

Track missing or partial language features. Use `[x]` when done, `[~]` for partial, `[ ]` for not started.

Legend matches `docs/OOP_CHECKLIST.md`: **done** · **partial** · **not started**

---

## Already called out or half-done

- [x] Integer modulus (`%`)
- [x] True wide-float arithmetic (1024-bit storage; limb-wise add/sub when upper limbs non-zero; f64 fast path for mul/div)
- [x] Wide integer multiply, divide, and modulus beyond 64 bits
- [x] Additional compound assignment (`*=`, `/=`, `^=`, `&=`, `|=`)

---

## Core language ergonomics

- [x] `nah` in `loop until` (exit loop or menu)
- [x] `goNext` in `loop until`
- [x] `menu` / `choose` multi-way branching on `int` or `bool`
- [x] Enums (`enum Name { A, B, C }` with auto or explicit values)
- [x] Limited type inference (e.g. infer type from RHS or literals)
- [x] `immportal` / immutability for bindings and cust fields

---

## Collections and data

- [x] First-class `Array<T>` for `int`, `float`, `str`, and stack `cust` types
- [x] Array length / `.len` or compile-time size helpers
- [x] For-each iteration over arrays
- [x] Distinct `char` type (documented convention: one-character `str` via `s[i]`)
- [x] Stronger compile-time bounds checking where possible

---

## OOP and generics

- [x] Generic methods on generic `cust` types
- [x] Stack-valued polymorphism (interfaces without requiring heap `adr`)
- [x] Multiple interfaces per type (`implements A, B`)
- [x] Generic templates with `implements` (checked after monomorphization)
- [x] Generic templates with `extends` (concrete base types only)
- [x] Operator overloading for `cust` types (`operator +`, `-`, `*`, `/`, `%`)

---

## Memory, I/O, and systems

- [x] File I/O (read/write files)
- [x] Standard input (stdin) support
- [x] Keyboard input without blocking (`KeyAvailable`, `PollKey`; line-buffered stdin — raw mode / signals not supported)
- [x] Wide heap peek/poke (`PeekI64`, `PokeI64` for 64-bit at offsets)
- [x] Shared / read-only borrows for `adr` (beyond move-only model)
- [x] Alternative allocators (arenas, pools — `ArenaCreate`, `arenaRent`, `PoolCreate`, `poolRent`)

---

## Tooling and platform

- [~] Cross-platform codegen and documented targets (Linux, x86_64, etc.) — macOS arm64/x86_64 universal binary; Linux x86_64 ELF with `_start` and gcc link; Linux aarch64 still host-only partial; `--list-targets` documents options
- [x] Standard library (`reference std.*` — `std.math`, `std.io`, `std.result`, `std.alloc`)
- [x] Module / namespace system (`reference qualified` — symbols mangled as `module__symbol`)
- [x] Richer compile diagnostics (type names, promotion hints; `--json-diagnostics` for tooling)
- [x] LSP features (go-to-def, rename, inline errors) — VS Code extension: `--check` on save/change with `--json-diagnostics`, go-to-definition, rename, document outline

---

## Higher level (longer term)

- [x] Structured errors (`Result` / `try` / typed error returns — `ResultInt` in `std.result`, `try {r} T = expr else { ... }`)
- [x] Closures / nested functions with captures — implicit capture-by-value at call site via hidden params on hoisted `outer__inner` symbols
- [x] Compile-time evaluation beyond `sizeof` (const int binops, enum refs; folded in visitor)
- [x] Full soft-float implementation for 128–1024 bit floats (limb-wise add/sub/mul/div when upper limbs non-zero; f64 IEEE fast path when upper limbs zero)
- [x] Tagged unions / sum types (`enum Option { None, Some(int) }` generates `Option` cust with `{tag}` / `{payload}`)

---

## Suggested priority

### High impact, relatively scoped

- [x] `%` (modulus)
- [x] `nah` / `goNext`
- [x] `Array<T>` polish (`Array<int|float|str|cust>` + `.len` + `foreach`)
- [x] File I/O and minimal std library (builtins: file, stdin, heap peek/poke)
- [x] Shared / read-only borrows (`&adr`, `&mut adr`)

### Medium effort, high value

- [x] Wide integer `*` / `/` / `%`
- [x] Wide-float storage with f64 fast path
- [x] `menu` / `choose` on int/bool (works with enum constants)
- [x] Simple enums + type inference + `immportal` bindings and fields
- [x] `PeekI64` / `PokeI64`
- [x] Non-blocking stdin helpers (`KeyAvailable`, `PollKey`)
- [x] Generic methods
- [x] Module namespacing
- [x] Compile-time array bounds (const indices)
- [x] Multiple interfaces per type
- [x] Stack-valued interface polymorphism
- [x] Generic template `implements`
- [x] Alternative allocators (arena + pool)
- [x] `std.*` library tree
- [x] Structured errors + `try`/`else`
- [x] Compile-time int expression folding
- [x] Generic template `extends`
- [x] Operator overloading
- [x] Tagged unions / sum types
- [x] Nested functions (no captures) — inline nested defs hoisted to `outer__inner` symbols
- [x] Compile diagnostics + `--check` mode

### Larger projects

- [x] Closures with captures (implicit; call-site by-value; no escaping/return yet)
- [x] Full 1024-bit soft-float (IEEE 754 soft-float for 64–1024 bit widths; f64 fast path when upper limbs zero; canonical encode/decode for wide IEEE ops)
- [x] Built-in GUI library, zero dependencies (`Gui*` builtins: window via raw Objective-C runtime on macOS, software rasterizer, embedded 8x8 bitmap font, mouse/keyboard events, BMP screenshots; Linux backend pending — see `example/gui_demo.minusc`, `example/chess_gui.minusc`)
- [~] Cross-platform runtime and CI (macOS universal binary; Linux codegen partial)
- [~] Rich compile diagnostics + LSP (inline errors on save; go-to-def/rename deferred)

---

## Test suites (`make test-all`)

| Target | Coverage |
|--------|----------|
| `test-borrow` | Move/`&adr`/`&mut adr`, scope, cust field borrows |
| `test-cust` | Stack `cust` types, init, fields |
| `test-oop` | Methods, `self`, visibility |
| `test-heap-oop` | Heap objects, `init`/`drop`, scope cleanup |
| `test-poly` | Inheritance, `virtual`, `super` |
| `test-generic` | Generic monomorphization, methods, template `implements` |
| `test-interface` | Interfaces, dual `implements`, stack interface params |
| `test-module` | `reference qualified` namespacing |
| `test-numeric` | Wide int/float, promotion |
| `test-feature` | `%`, compound assign, `menu`, `nah`/`goNext` |
| `test-io` | File I/O, path security, stdin |
| `test-new-feature` | Enums, inference, `immportal`, const array bounds |
| `test-advanced` | constexpr fold, `try`/`else`, arena/pool, `std.*` |
| `test-remaining` | generic `extends`, operator overload, sum types, nested fn, closures, diagnostics |

---

## Related docs

- [README.md](../README.md) — current language reference
- [docs/OOP_CHECKLIST.md](./OOP_CHECKLIST.md) — OOP implementation status (Phases 1–5 complete)

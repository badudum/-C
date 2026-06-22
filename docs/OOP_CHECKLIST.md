# minusC OOP Implementation Checklist

Track progress on the object-oriented features for minusC. The pipeline is organized into five phases; **Phases 1–5 are complete**.

**Legend:** `[x]` done · `[~]` partial · `[ ]` not started

---

## Pipeline at a glance

| Phase | Goal | Progress |
|-------|------|----------|
| **1** | Structs (`cust`) — data-only composite types | **complete** |
| **2** | Methods + encapsulation | **complete** |
| **3** | Heap objects + lifetimes | **complete** |
| **4** | Inheritance + virtual dispatch | **complete** |
| **5** | Generics / interfaces | **complete** |

User-facing syntax and examples for OOP (including Phase 5): **`README.md`** (sections *Custom types*, *Generic types*, *Interfaces*).

**Recommended order:** finish Phase 1 polish → Phase 2 (methods, `self`, visibility) → Phase 3 (heap `self`, `init`/`drop`) → Phase 4 (polymorphism) → Phase 5 (generics/interfaces).

**Open design decisions (decide before Phase 2):**

- [x] Methods inside `cust { }` block (chosen over separate free functions)
- [x] `self` by-value (stack) and `adr` (heap) — both supported; heap for polymorphic dispatch
- [x] Single inheritance only for v1 (recommended: yes)
- [x] Static dispatch by default; `virtual` opt-in (recommended: yes)

---

## Phase 1 — User-defined types (`cust`)

Foundation layer: struct-like types without methods.

### Core type system

- [x] `cust` keyword in lexer (`CUST_TOKEN`)
- [x] Type definitions: `Point = cust { {x} int; {y} int; };`
- [x] Field layout, alignment, and byte offsets (`src/utils/cust.c`)
- [x] Type registry and lookup by name (`cust_lookup_by_name`, `cust_get`)
- [x] `TYPE_CUST` encoding in type system (`MAKE_CUST_TYPE`, `IS_CUST_TYPE`)
- [x] `sizeof(Point)` for custom types (`TYPE_SIZE_AST`, visitor, assembly)

### Initialization

- [x] Typed initializer: `Point{x=3, y=4}`
- [x] Bare brace initializer: `{p} Point = {x=10, y=5};`
- [x] Nested initializer: `Rect{origin=Point{x=1, y=2}, w=30, h=40}`
- [x] Field-order validation in `visit_cust_init`

### Field access and assignment

- [x] Field read: `p.x`, nested `r.origin.x`
- [x] Field write: `p.x = 20`
- [x] `FIELD_ACCESS_AST` in parser, visitor, assembly
- [x] Copy assignment between same `cust` type: `{b} Point = a;`
- [x] Type mismatch errors for `cust` assign/init

### Supported field types

- [x] `int`
- [x] `bool`
- [x] `str`
- [x] `adr`
- [x] `Array<T>` (element type required)
- [x] Nested `cust` types

### Storage and codegen

- [x] Stack allocation (multi-slot layout via `cust_type_slots`)
- [x] Field load/store at computed FP offsets (`cust_fp_offset`)
- [x] Cust init and copy in assembly (`append_init_cust_from_literal`, `append_copy_cust_fields`)
- [x] Visitor integration (`visit_cust_def`, `visit_cust_init`, `visit_field_access`)

### Tests and docs

- [x] `example/cust_tests.minusc` (10 positive tests)
- [x] `example/cust_fail/` negative compile tests
- [x] `scripts/run_cust_tests.sh` + `make test-cust`
- [x] Wired into `example/main.minusc`
- [x] README section for custom types (`cust`)

### Phase 1 gaps / polish

- [x] Fix nested compound-init codegen (parent field offset in `append_copy_cust_fields_at`)
- [x] Standalone `{ }` statement blocks (scoped compounds in parser + visitor)
- [x] `rent(1, Point)` smoke test as heap bytes
- [x] `class` keyword alias for `cust`
- [x] Array-field ergonomics: `Array<T>` required; negative test for bare `Array`

**Phase 1 exit criteria:** met — nested init, class keyword, blocks, heap rent, and Array fields covered by tests.

---

## Phase 2 — Methods + encapsulation

Real OOP ergonomics: behavior attached to types.

### Syntax and parsing

- [x] Method definitions inside `cust` / `class` body
- [x] Instance method call syntax: `obj.method()` or `obj.method(args)`
- [x] `self` receiver parameter (naming and placement convention)
- [ ] Static methods (no `self`) — deferred for v1
- [x] `AST` nodes for method defs and method calls (or desugar early to mangled calls)

### Symbol table and resolution

- [x] Per-type method table (name → function, arity, return type)
- [x] Method lookup on static type of receiver
- [x] Name mangling convention (e.g. `Point_lenSq`, `Point_init`)
- [x] Desugar `v.lenSq()` → `Point_lenSq(&v)` (or equivalent)

### Visibility

- [x] `public` / `private` keywords (lexer + parser)
- [x] Visibility stored on fields and methods in type metadata
- [x] Compile-time rejection of cross-type private access
- [ ] `protected` (defer until Phase 4 inheritance)

### Construction and teardown (conventions)

- [x] Named constructor / `init` method convention
- [x] `drop` method convention for resource cleanup (manual + scope auto-drop for heap objects)
- [x] Document that literal init remains valid alongside `init`

### Visitor and type checking

- [x] Type-check method calls (receiver type, arg types, return type)
- [x] Type-check `self` field access inside methods
- [x] Reject invalid `self` use (if applicable)

### Codegen

- [x] Pass receiver as first argument (stack slot or `adr`)
- [x] Emit mangled function symbols for methods
- [x] Method calls use existing call ABI

### Tests

- [x] Positive: define type with method, call `obj.method()`
- [x] Positive: `init` sets fields; `lenSq`-style instance method
- [x] Negative: private field access from outside type
- [x] Negative: wrong receiver type on method call (`missing_method.minusc`)
- [x] Wire into `main.minusc` or dedicated `oop_tests.minusc` (`make test-oop`)

**Phase 2 exit criteria:** met — stack `cust` values support methods, `self`, and visibility; `make test-oop` passes.

---

## Phase 3 — Heap objects + ownership

Align objects with `rent` / `moveOut` and the borrow checker.

### Heap object model

- [x] `rent(1, MyType)` as idiomatic heap object allocation
- [x] Object size = `sizeof(MyType)` via existing `rent(count, type)` path
- [x] `init(self, ...)` for heap objects (write fields through `adr`)
- [x] `drop(self)` before / with `moveOut` for types that own `adr` fields

### Methods on heap `self`

- [x] `self` as `adr` pointing at heap object layout
- [x] Field access through `self` (`self.x` → offset from heap base)
- [x] Method calls on heap handles: `p.method()` when `p` is `adr`

### Borrow checker integration

- [x] Borrow rules for `self` as owner of inner `adr` fields
- [x] Move semantics when transferring heap object ownership
- [x] Cannot use `self` after `moveOut(self)` (mirror `adr` rules)
- [x] Loans on fields accessed through heap `self` (extend `var.field` keys if needed)

### Lifetime (optional enhancements)

- [x] Scope-based auto-`drop` at block end for leaked heap objects
- [x] Document manual `drop` + `moveOut` as the v1 pattern

### Tests

- [x] Positive: `rent(1, T)` + `init` + method + `drop` + `moveOut`
- [x] Positive: `cust` with `adr` field on heap object
- [x] Negative: use after `moveOut` on heap object handle
- [x] Negative: double `drop` / double `moveOut`
- [x] Borrow tests for heap `self` (`example/heap_oop_tests.minusc` heap-5; `make test-heap-oop`)

**Phase 3 exit criteria:** met — heap-backed objects work end-to-end with ownership and borrow rules; `make test-heap-oop` passes.

---

## Phase 4 — Polymorphism

Classical inheritance and dynamic dispatch.

### Inheritance

- [x] `extends` keyword and syntax: `Dog = cust extends Animal { ... };`
- [x] Subobject layout (base fields first, derived fields after)
- [x] Constructor chaining / `super.init(...)`
- [x] Upcasting: pass `Dog` where `Animal` is expected (layout-compatible `adr` or stack slice)

### Method overriding

- [x] Override detection (same method name/signature in subclass)
- [x] `virtual` keyword on overridable methods
- [x] Non-virtual by default; `init` may override without `virtual`

### Virtual dispatch

- [x] Vtable layout per class with virtual methods
- [x] Vtable pointer in heap object header (8 bytes before fields)
- [x] Codegen: `obj.speak()` → load vtable → indirect call (heap receivers)
- [x] `super.method()` calls parent implementation (static dispatch)

### Tests

- [x] Positive: override called on runtime type
- [x] Positive: `super` calls base method
- [x] Negative: invalid override (signature mismatch)
- [x] Negative: access `private` base field from subclass (if visibility + inheritance interact)

**Phase 4 exit criteria:** met — single inheritance + `virtual` + vtables work with tests (`make test-poly`).

---

## Phase 5 — Generics / interfaces

Scale the type system (defer until Phases 2–4 are stable).

### Generics

- [x] Type parameters on `cust` / `class`: `Box<T> = cust { {value} T; };`
- [x] Monomorphization strategy (concrete types like `Box_int`, `Box_Point`)
- [x] Generic method support (`get()` etc. on monomorphized types)

### Interfaces / traits

- [x] `interface` definition syntax
- [x] `implements` on types
- [x] Interface satisfaction checking (nominal; virtual methods must match vtable slots)
- [x] Multiple interfaces per type (`implements A, B`)
- [x] Stack-valued interface params (stack `cust` without `rent`)
- [x] Generic template `implements` (checked at monomorphization)
- [x] Dispatch model (interface params erase to `adr`; virtual dispatch via vtable slot; stack args copied to vtable buffer at call site)

### Tests

- [x] Positive: `Box<int>` and `Box<Point>` distinct monomorphized instances
- [x] Positive: generic method on `Box<int>`
- [x] Positive: generic template `implements` + stack interface call
- [x] Positive: dual-interface virtual dispatch
- [x] Positive: stack `cust` through interface param
- [x] Negative: missing interface method

**Phase 5 exit criteria:** met — generic monomorphization and interface `implements` + param checking covered by tests (`make test-generic`, `make test-interface`).

---

## Supporting infrastructure (already in place)

These are not OOP features but reduce work for later phases.

- [x] `sizeof(type)` — primitives, `Array<T>`, `cust`
- [x] `rent(bytes)` and `rent(count, type)` — typed heap allocation
- [x] `moveOut`, `PeekInt`/`PokeInt`, `Memcpy`, `Memset`, `RentGrow`
- [x] Borrow checker — move semantics, `&adr` / `&mut adr`, block/`if` scope, per-field `cust` `adr`
- [x] Free functions as interim behavior (`takeAdr(buf)`, etc.)
- [x] Compiler pipeline: preprocess → lexer → parser → visitor → assembly → link
- [x] Cross-compilation: universal `minusC.out` (arm64 + x86_64)
- [x] Integration test driver: `example/main.minusc`
- [x] Focused suites: `make test-all` (borrow, cust, oop, heap-oop, poly, generic, interface, module, numeric, feature, io, new-feature)

---

## Compiler touchpoints (reference)

When implementing OOP, expect changes across:

| Stage | Phase 1 | Phase 2+ |
|-------|---------|----------|
| **Lexer** | `cust` | `class`, `extends`, `virtual`, `public`, `private`, `self`, `super`, `interface` |
| **Parser** | cust def, init, field access | method defs, `obj.method()` calls |
| **AST** | `CUST_DEF_AST`, `CUST_INIT_AST`, `FIELD_ACCESS_AST` | method def/call nodes (or desugar) |
| **Symbol table** | `cust` registry, fields | methods, vtables, visibility |
| **Visitor** | layout, field types, init | method resolution, visibility, `self` typing, borrow on `self` |
| **Assembly** | field offsets, copy/init | vtable emit, indirect calls, heap `self` offsets |
| **Runtime** | (none extra) | minimal vtable data; reuse `rent`/`moveOut` |

### Semantic analyses to add (Phase 2+)

- [x] Layout pass — field offsets + vtable header on heap objects
- [x] Method resolution — find method for receiver static type
- [x] Override checking — when `virtual` + `extends` exist
- [x] Visibility checking — public/private boundaries
- [x] Borrow on `self` — moves and loans through receiver

---

## Minimum viable OOP (target)

Check all of these for a usable v1 OOP story:

- [x] User-defined types with fields (`cust`)
- [x] Construction via literals / field init
- [x] Field access and assignment
- [x] Methods with `self`
- [x] `init` / `drop` lifecycle hooks
- [x] `public` / `private` encapsulation
- [x] Heap objects via `rent` + ownership
- [x] Inheritance + virtual dispatch
- [x] Generics (monomorphization) + interfaces
- [x] Module namespacing (`reference qualified`)

---

## Deferred / out of scope for v1

- [ ] Multiple inheritance
- [ ] Operator overloading on types
- [ ] Reflection
- [ ] Exceptions in constructors
- [ ] Property getters/setters (syntactic sugar)
- [ ] Static fields with global mutable state
- [ ] Cross-function borrow lifetimes (general Rust-style lifetimes)

---

## Related files

| Area | Path |
|------|------|
| Cust registry | `src/utils/cust.c`, `src/include/cust.h` |
| Parser | `src/parser.c` |
| Visitor | `src/visitor.c` |
| Assembly | `src/assembly.c` |
| Types | `src/include/types.h`, `src/utils/types.c` |
| Borrow + cust `adr` | `src/utils/borrow.c` |
| Cust tests | `example/cust_tests.minusc` |
| Borrow tests | `example/borrow_tests.minusc` |
| Language overview | `README.md` |

---

*Last updated to reflect Phases 1–5, generic methods, dual interfaces, stack interface params, module namespacing, and `make test-all`.*

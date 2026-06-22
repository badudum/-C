# Phase 2 — open design questions

## Phase 3 — heap objects (implemented)

### Manual `drop` + `moveOut` (v1 pattern)

1. Allocate: `{p} adr = rent(1, MyType);`
2. Initialize: `p.init(...);` — methods receive `self` as an `adr` to the heap layout.
3. Use methods / field access on the handle: `p.method()`, shared loans on `p.data`, etc.
4. **Before** releasing the heap block, call `p.drop();` when the type owns inner `adr` fields (`moveOut(self.data)` inside `drop` is typical).
5. Release the outer handle: `moveOut(p);`

There **is** scope-based auto-`drop` + `moveOut` at the end of braced `{ ... }` blocks for heap-tagged `rent(1, T)` handles. Manual `drop()` before block exit is respected (no second `drop`; `moveOut` still runs if the handle was not moved).

### Heap handle tagging

- `rent(1, T)` tags the `adr` variable with the cust type id (`int_value` on the assignment).
- Method receivers use a unified `self`-as-`adr` ABI (stack cust callers pass the address of the stack object; heap handles pass the heap pointer).
- Field access on heap handles and `self` uses `CUST_ACCESS_HEAP` (load/store via base pointer + offset).

### Tests

- `make test-heap-oop` — positive suite + `example/heap_oop_fail/` (use-after-moveOut, double moveOut, use-after-drop field).
- Existing `make test-oop`, `make test-cust`, `make test-borrow` remain green.

---

Items we punted or approximated for v1. Please weigh in when convenient.

## Receiver ABI

- **Stack `cust` vs heap `adr`:** Methods pass the receiver as the first call argument using the same stack-slot ABI as ordinary functions. `rent(1, T)` heap values are not wired as method receivers yet — should `self` on a heap-backed instance be an `adr`, or do we copy in/out?
- **Multi-slot `cust` types:** Receivers wider than one machine word (e.g. nested `Rect`) are only lightly tested. Confirm whether the caller should spill multiple slots in declaration order.

## Methods

- **Static methods (no `self`):** Skipped for v1; only `(self) function { ... }` is accepted inside `cust` / `class`.
- **Method overloads:** Duplicate method names are rejected; no arity-based overloading.
- **`drop` lifecycle:** No automatic `drop()` call yet — only the naming convention is reserved. Should literal init skip `drop`, and who invokes it (scope exit, reassignment, `moveOut`)?

## Visibility

- **`protected`:** Deferred to Phase 4 (inheritance). Same-type access only for `private` today.
- **Friend / package visibility:** Not planned unless requested.

## Naming / builtins

- **`HowBig(T)`** aliases `sizeof(T)` for the lighthearted builtin style. Keep both forever, or migrate docs/examples to `HowBig` only?
- Future builtins (`TypeOf`, `CloneMe`, …) — prefer PascalCase verb phrases like `HelloWorld` / `PeekInt`?

## Parser / ergonomics

- **Block shadowing:** Same as Phase 1 — `{outer}` redeclaration inside a nested block is still fragile; methods do not change that.
- **`init` return type:** `init` is a normal method; caller may ignore the `int` return. Should `init` be `void`-only sugar?

## Testing gaps

- No runtime test for private method on a *different* type in the same file (only compile-fail).
- No `drop` or static-method tests until decisions above land.

## Discovered ergonomics (from complex tests)

- **Method definition order:** a method that calls `self.other()` requires `other` to appear *earlier* in the `cust` body (visitor registers methods in source order).
- **Chained calls on `self`:** use `{tmp} int = self.bumpX();` rather than a bare `self.bumpX();` statement if you need side effects before another call.
- **`if` / `loop until` inside method bodies:** currently unreliable in methods; prefer delegating control flow to a module-level helper (see `triSumFrom` in `example/oop_complex.minusc`).
- **Method call in `return` binop:** `return self.x + self.peekY()` is fragile; assign the call result to a local first.

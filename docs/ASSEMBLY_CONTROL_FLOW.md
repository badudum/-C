# Assembly: If, Loops, Booleans — Construction and Stack Frame

This document explains how the minusC compiler lowers **if/else**, **loop until** (while/for/do-while), and **booleans** to ARM64 assembly, and how the **stack frame** is used. There is no heap allocation for these; everything is stack-based.

---

## 1. Stack frame layout

### 1.1 Role of `fp` and `sp`

- **`sp`** (stack pointer): current top of stack; moves when we `sub sp, sp, #N` or `str x0, [sp, #-16]!`.
- **`fp`** (frame pointer, **x29**): set once in the function prologue and left fixed for the whole function. All local variables and temporaries are addressed **relative to `fp`**.

So the “stack frame” is the region of stack reserved for the function and addressed via `fp`.

### 1.2 Function prologue (from `function_ass.h` / `function.asm`)

```asm
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #<frame_size>
```

- Push link register (x30) and old frame pointer (x29), then set **x29 = sp** (this becomes our **fp**).
- Then **sp** is decreased by **frame_size** to reserve space for locals and temporaries.

So after the prologue:

- **fp** = value of **sp** at the moment we did `mov x29, sp` (so fp is “above” the reserved block).
- **sp** = fp − frame_size (lower addresses).

Locals live in the block between **sp** and **fp** (at lower addresses than **fp**).

### 1.3 How slots map to memory: `stack_index` and offsets

The **visitor** assigns each variable and each temporary result a **stack_index** (1, 2, 3, …) by pushing a placeholder onto `stackframe->stack`. The assembler then uses:

- **Offset from fp:**  
  `offset = stack_index * (-16)`  
  So slot 1 → `[fp, #-16]`, slot 2 → `[fp, #-32]`, etc.

- **Frame size:**  
  `(1 + stackframe->stack->size) * 16`  
  so there is enough 16-byte-aligned space for all slots.

So:

- **Stack frame** = one contiguous block of stack reserved in the prologue.
- **No heap** is used for if/loops/booleans; only this fp-relative stack space.
- **Load:** `ldr w0, [fp, #offset]` (or, for large |offset|, `sub x4, fp, #|offset|` then `ldr w0, [x4]`). For booleans we use **`ldrb w0`** so only one byte is read.
- **Store:** `str w0, [fp, #offset]` (or same two-instruction sequence for large offsets). For booleans we use **`strb w0`** so only one byte is written.

---

## 2. Booleans

Booleans are **0 = Fake**, **1 = Real**. Semantically they are one-bit values; in memory we store them in **one byte** per value (not a full word) for efficiency.

- **Storage:** Each boolean still gets its own **stack slot** (same 16-byte-aligned slot layout as other locals), but we only **read/write one byte** at that slot using **`ldrb`** / **`strb`** instead of `ldr` / `str`. So we use 1 byte of the slot instead of 4.
- **In registers:** After load, the value is in **w0** (zero-extended by `ldrb`), so it is still 0 or 1 and works with `cbz`/`cbnz` and comparisons.

### 2.1 Boolean literals (`Real` / `Fake`) — `assemble_bool`

- **Stack:** The visitor has already assigned a slot (`stack_index`); the bool is stored there (1 byte).
- **Code:**
  - Push so the rest of the codegen is consistent: `str x0, [sp, #-16]!`
  - Put the value (0 or 1) in **w0**, then store **one byte** at the slot:  
    `mov w0, #0` or `mov w0, #1`  
    `strb w0, [fp, #<offset>]`  
    (or `sub x4, fp, #<abs_offset>`; `strb w0, [x4]` for large offsets)

So the “construction” is: **reserve slot (visitor) → emit byte store of 0/1 at that slot (assembler)**.

### 2.2 Comparisons (`==`, `!=`, `<`, `>`, `<=`, `>=`) — `assemble_binop`

- **Stack:** Left and right operands are already in their slots; the result gets a **new** slot (assigned when the comparison node was visited).
- **Code:**
  1. Load left into **w0**, right into **w1** (use **`ldrb`** when the operand is a boolean, else `ldr`).
  2. Compare:  
     `cmp w0, w1`
  3. Set **w0** to 1 if condition true, 0 otherwise:  
     `cset w0, <cond>`  
     where `<cond>` is an ARM64 condition (e.g. `eq`, `ne`, `lt`, `gt`, `le`, `ge`).
  4. Store the boolean result in the result slot (use **`strb`** for the bool result):  
     `strb w0, [fp, #<result_offset>]`

So the “construction” is: **two loads (byte when bool) → cmp → cset (boolean 0/1) → byte store**. All in fp-relative stack; no heap.

### 2.3 Logical AND/OR — `assemble_binop` (AND_TOKEN / OR_TOKEN)

- **Stack:** Again, left/right in their slots, result in a new slot.
- **Code:**
  1. Load left and right into **w0** and **w1** (use **`ldrb`** when the operand is a boolean).
  2. Turn each into 0/1:  
     `cmp w0, #0`  
     `cset w0, ne`  
     (and same for w1).
  3. **AND:** `and w0, w0, w1`  
     **OR:** `orr w0, w0, w1`
  4. Store **w0** at the result slot (use **`strb`** for the bool result).

So logical ops are: **normalize to 0/1 with cset, then bitwise and/or, then store**.

### 2.4 Logical NOT — `assemble_unary` (NOT_TOKEN)

- **Stack:** Operand in one slot, result in another (both assigned in the visitor).
- **Code:**
  1. Load operand: `ldr w0, [fp, #<operand_offset>]`
  2. Compare with 0 and set 1 if equal (i.e. “not”):  
     `cmp w0, #0`  
     `cset w0, eq`
  3. Store: `str w0, [fp, #<result_offset>]`

So: **load → cmp #0; cset eq → store**. All stack.

---

## 3. If / else if / else — `assemble_if`

Control flow is done with **labels** and **conditional/unconditional branches**. The condition is a **boolean in a stack slot** (from a comparison, logical op, or literal); we load it into **w0** and branch on it.

### 3.1 Labels

- **`_endif_<id>`** — after the whole if/else; we jump here to skip the “then” or “else” when needed.
- **`_else_<id>`** — start of the else (or else-if) block; we jump here when the condition is false.

`id` comes from a static **if_label_counter** so each if gets unique labels.

### 3.2 Assembly sequence (conceptually)

1. **Condition**
   - Assemble the condition expression (e.g. comparison); it leaves its result in a slot.
   - Load that slot into **w0**:  
     `ldr w0, [fp, #<cond_offset>]`
   - If there is an **else**:  
     `cbz w0, _else_<id>`  
     (branch if **w0** is zero → take the else).
   - If there is **no else**:  
     `cbz w0, _endif_<id>`  
     (branch if zero → skip the then block).

2. **Then block**
   - Assemble all children of the if node (the “then” body).
   - Then:  
     `b _endif_<id>`  
     (so we don’t fall into the else).

3. **Else block** (if present)
   - Emit:  
     `_else_<id>:`
   - Assemble the else (which may be another if, so `assemble_if` is recursive).
   - Execution then falls through to the next code (or we’re already inside a larger if’s endif).

4. **End of if**
   - Emit:  
     `_endif_<id>:`  
     (rest of the program continues from here).

So the “construction” is: **condition code → load condition → cbz to else/endif → then code → b endif → [else label + else code] → endif label**. All condition value lives in the stack; no heap.

### 3.3 Stack frame usage in if

- The **condition** is an expression (e.g. `a < b`). Its operands and result get **stack_index** and slots like any other expression.
- The **then** and **else** bodies are just more statements; their variables and temporaries also get slots in the same frame.
- So: **one frame** for the whole function; if/else only add **control flow** (labels + branches), not extra frame allocation.

---

## 4. Loops — `assemble_loop_until`

Loops are implemented with **labels** and **conditional (cbz/cbnz) / unconditional (b) branches**. The same stack frame is used for the whole function; loop bodies do not allocate a new frame.

### 4.1 Labels

- **`_loop_cond_<id>`** — where we evaluate the condition (used for while and for).
- **`_loop_body_<id>`** — start of the loop body (used for do-while).
- **`_loop_end_<id>`** — after the loop; we jump here when the condition is false.

`id` comes from **loop_label_counter**.

### 4.2 While-style: `loop until (condition) { body };`

- **Stack:** Condition and body use the same frame; condition result and body variables all have slots.
- **Sequence:**
  1. `_loop_cond_<id>:`
  2. Assemble condition → leaves result in a slot.
  3. Load condition into **w0**: `ldr w0, [fp, #<cond_offset>]`
  4. `cbz w0, _loop_end_<id>` — if false, exit loop.
  5. Assemble **body**.
  6. `b _loop_cond_<id>` — repeat.
  7. `_loop_end_<id>:`

So: **cond label → condition code → load → cbz to end → body → b back to cond → end label**.

### 4.3 For-style: `loop until (init; condition; step) { body };`

- **Stack:** Init, condition, step, and body all use the same frame; each gets its own slots (e.g. loop variable from init, condition result, etc.).
- **Sequence:**
  1. Assemble **init** once (e.g. `i = 0` or `{i} int = 0`).
  2. `_loop_cond_<id>:`
  3. Assemble **condition** → load into **w0** → `cbz w0, _loop_end_<id>`.
  4. Assemble **body**.
  5. Assemble **step** (e.g. `i++`).
  6. `b _loop_cond_<id>`
  7. `_loop_end_<id>:`

So: **init (once) → cond label → condition → cbz end → body → step → b cond → end label**.

### 4.4 Do-while-style: `loop { body } until (condition);`

- **Stack:** Same idea: one frame; condition and body use slots in that frame.
- **Sequence:**
  1. `_loop_body_<id>:`
  2. Assemble **body**.
  3. Assemble **condition** → load into **w0**.
  4. `cbnz w0, _loop_body_<id>` — if **true**, go back to body.
  5. `_loop_end_<id>:`

So: **body label → body → condition → cbnz back to body → end label**.

(Do-while with a for-style clause is a combination: body once, then init, then the same for-loop structure as above.)

### 4.5 Stack frame usage in loops

- **No extra stack allocation** for the loop itself; the same **fp** and the same slot layout are used.
- Loop variables (e.g. `i` from `{i} int = 0` or `i = 0`) and condition results are just more slots in that frame.
- So: **loops only add labels and branches**; all data stays in the existing stack frame.

---

## 5. Summary table

| Construct        | Stack usage                          | Control flow |
|-----------------|--------------------------------------|--------------|
| Boolean literal | One slot at `[fp, #-16*stack_index]` | None         |
| Comparison      | Left/right slots → result slot       | None         |
| Logical and/or  | Left/right slots → result slot       | None         |
| Logical not     | Operand slot → result slot           | None         |
| If / else       | Condition and bodies use same frame  | cbz, b, labels _else_<id>, _endif_<id> |
| While           | Cond + body in same frame             | _loop_cond_<id>, cbz, b, _loop_end_<id> |
| For             | Init/cond/step/body in same frame     | init once, _loop_cond_<id>, cbz, body, step, b, _loop_end_<id> |
| Do-while        | Body + cond in same frame             | _loop_body_<id>, body, cond, cbnz, _loop_end_<id> |

- **Heap:** Not used for if, loops, or booleans; only the **stack** (and fp-relative addressing) is used.
- **Stack frame:** One per function; **fp** and **stack_index → [fp, #-16*stack_index]** define all locals and temporaries for that function, including those used inside if/else and loops.

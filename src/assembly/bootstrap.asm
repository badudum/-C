HelloWorld:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #32
    mov x6, x0
    bl strlen
    mov x2, x0
    mov x1, x6  ; x1 now contains the string address
    mov x16, #4
    mov x0, #1
    svc #0

    mov x16, #1
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

strlen:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov w1, #0
    mov x3, x0  ; Use x0 as the input argument
    cbz x3, strlenend

strlenloop:
    ldrb w2, [x3, w1, uxtw #0]
    cmp w2, #0
    beq strlenend
    add w1, w1, #1
    b strlenloop

strlenend:
    mov x0, x1
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

itos:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    ldr w0, [sp, #16]
    mov w1, #8
    add x2, sp, w1, uxtw #1
    mov w1, #0
    mov w3, #0
    str wzr, [sp, #-8]!
    b itos_loop

itos_loop:
    mov w2, #0
    mov w4, #10
    udiv w0, w0, w4
    add w2, w2, #48
    str w2, [sp, #-8]!
    add w1, w1, #1
    cbz w0, itos_buffer_loop
    b itos_loop

itos_buffer_loop:
    ldr w2, [sp], #8
    strb w2, [x2, w3, uxtw #0]
    cbz w1, itos_end
    add w3, w3, #1
    sub w1, w1, #1
    b itos_buffer_loop

itos_end:
    mov x0, x2
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

return_statement:
    ldp x29, x30, [sp], 0x10
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret
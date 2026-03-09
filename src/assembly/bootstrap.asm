HelloWorld:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #32
    mov x6, x0
    bl strlen
    mov x2, x0
    mov x1, x6
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
    mov x3, x0
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
    sub sp, sp, #32
    mov x2, sp
    add x3, x2, #31
    strb wzr, [x3]
itos_loop:
    mov w4, #10
    udiv w5, w0, w4
    msub w6, w5, w4, w0
    add w6, w6, #48
    sub x3, x3, #1
    strb w6, [x3]
    mov w0, w5
    cbnz w0, itos_loop
    mov x0, x3
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

HelloWorldLine:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    sub sp, sp, #16
    mov w0, #0x0a
    strb w0, [sp]
    strb wzr, [sp, #1]
    mov x0, sp
    bl HelloWorld
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

return_statement:
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

.section .text
.global _start
_start:
    mov fp, sp
    bl main
    mov r7, r0
    mov r0, #1
    svc #0# compound (0x6503b61fe620) 
HelloWorld:
    push {fp, lr}
    add fp, sp, #4
    ldr r0, [fp, #8]
    bl strlen
    add sp, sp, #4
    ldr r1, [fp, #8]
    mov r2, r0
    mov r0, #4
    mov r7, #1
    mov sp, fp
    pop {fp, lr}
    svc #0
    bx lr

.type itos, %function
itos:
    push {fp, lr}
    add fp, sp, #4

    ldr r0, [fp, #12]           # number
    mov r1, #8
    add r3, sp, r1, lsl #1      # buffer
    mov r1, #0                  # counter
    mov r2, #0

    push {r0}
    b itos_loop

itos_loop:
    mov r2, #0
    mov r3, #10
    udiv r0, r0, r3
    add r2, r2, #48
    push {r2}
    add r1, r1, #1
    cmp r0, #0
    beq itos_buffer_loop
    b itos_loop

itos_buffer_loop:
    pop {r2}
    strb r2, [r3, r1]
    cmp r1, #0
    beq itos_end
    add r1, r1, #1
    sub r1, r1, #1
    b itos_buffer_loop

itos_end:
    mov r0, r3
    mov sp, fp
    pop {fp, lr}
    bx lr

return_statement:
    pop {r0}
    mov sp, fp
    pop {fp, lr}
    bx lr

.type strlen, %function
strlen:
    push {fp, lr}
    add fp, sp, #4
    mov r1, #0
    ldr r0, [fp, #8]
    b strlenloop

strlenloop:
    ldrb r2, [r0, r1]
    cmp r2, #0
    beq strlenend
    add r1, r1, #1
    b strlenloop

strlenend:
    mov r0, r1
    mov sp, fp
    pop {fp, lr}
    bx lr
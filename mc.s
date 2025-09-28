.text
.globl _start
_start:
mov sp, fp
bl main
mov x1, x0
mov x0, #1
svc #0
 # compound (0x6000000a8d80) 
# start of "print"
.globl print
print:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #80
sub sp, sp, #96

 # compound (0x6000000a8f00) 

# variable (test)
str x0, [fp, #-24]
str x0, [sp, #-16]!

# call arg
ldr x0, [fp, #-48]
bl HelloWorld
add sp, sp, #0
str x0, [sp, #-16]!

# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
str x2, [fp, #0]


b return_statement
# assign default
add x0, sp, #0
str x0, [fp, #-0x10]

# variable (int)
str x0, [fp, #0]
str x0, [sp, #-16]!
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #240
sub sp, sp, #256

 # compound (0x6000000a9140) 

# Hello
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x06f6c6c6548
str x0, [sp, #0]

 add x0, sp, #0
str x0, [fp, #40]
# assign default
add x0, sp, #0
str x0, [fp, #-0x50]

# Taeyang Hi!Hello This is a longer string with newline
sub sp, sp, #64

mov x0, #0
str x0, [sp, #56]
ldr x0, =0x0656e696c77
str x0, [sp, #48]
ldr x0, =0x0656e206874697720
str x0, [sp, #40]
ldr x0, =0x0676e697274732072
str x0, [sp, #32]
ldr x0, =0x065676e6f6c206120
str x0, [sp, #24]
ldr x0, =0x07369207369685420
str x0, [sp, #16]
ldr x0, =0x06f6c6c6548216948
str x0, [sp, #8]
ldr x0, =0x020676e6179656154
str x0, [sp, #0]

 add x0, sp, #0
str x0, [fp, #104]
# assign default
add x0, sp, #0
str x0, [fp, #-0x70]

# David Hello
sub sp, sp, #16
ldr x0, =0x06f6c6c
str x0, [sp, #8]
ldr x0, =0x06548206469766144
str x0, [sp, #0]

 add x0, sp, #0
str x0, [fp, #80]
# assign default
add x0, sp, #0
str x0, [fp, #-0x90]

# whatwha
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x061687774616877
str x0, [sp, #0]

 add x0, sp, #0
str x0, [fp, #88]
# assign default
add x0, sp, #0
str x0, [fp, #-0xb0]

# variable (what)
str x0, [fp, #-88]
str x0, [sp, #-16]!

# call arg
ldr x0, [fp, #-176]
bl HelloWorld
add sp, sp, #0
str x0, [sp, #-16]!

# variable (name)
str x0, [fp, #-56]
str x0, [sp, #-16]!

# call arg
ldr x0, [fp, #-112]
bl HelloWorld
add sp, sp, #0
str x0, [sp, #-16]!

# variable (name)
str x0, [fp, #-56]
str x0, [sp, #-16]!

# call arg
ldr x0, [fp, #-112]
bl print
add sp, sp, #0
str x0, [sp, #-16]!

# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
str x2, [fp, #0]


b return_statement
# assign default
add x0, sp, #0
str x0, [fp, #-0x30]

# variable (int)
str x0, [fp, #0]
str x0, [sp, #-16]!
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
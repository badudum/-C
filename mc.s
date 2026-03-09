.text
.globl _start
_start:
bl main
mov x0, #0
mov x16, #1
svc #0

 # compound (0x600001b8b180) 
# start of "print"
.globl print
print:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #80

sub sp, sp, #96

# load parameter test from x0
str x0, [fp, #-48]

 # compound (0x600001b8b300) 

# variable (test)
ldr x0, [fp, #-48]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-48]
bl HelloWorld

# store return value
str x0, [sp, #-16]!

# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #30
str x2, [fp, #-80]

b return_statement
# assign default
add x0, sp, #0
str x0, [fp, #-0x10]

# variable (int)
ldr x0, [fp, #0]
str x0, [sp, #-16]!
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #1360

sub sp, sp, #1376

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001b8b540) 

# Hello
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x6f6c6c6548
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-64]
# assign default
add x0, sp, #0
str x0, [fp, #-0x50]

# Taeyang Hi!Hello This is a   longer string with newline   haha 
sub sp, sp, #64
ldr x0, =0x2061686168200a
str x0, [sp, #56]
ldr x0, =0x20656e696c77656e
str x0, [sp, #48]
ldr x0, =0x206874697720676e
str x0, [sp, #40]
ldr x0, =0x6972747320726567
str x0, [sp, #32]
ldr x0, =0x6e6f6c2009206120
str x0, [sp, #24]
ldr x0, =0x7369207369685420
str x0, [sp, #16]
ldr x0, =0x6f6c6c6548216948
str x0, [sp, #8]
ldr x0, =0x20676e6179656154
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-96]
# assign default
add x0, sp, #0
str x0, [fp, #-0x70]

# David Hello
sub sp, sp, #16
ldr x0, =0x6f6c6c
str x0, [sp, #8]
ldr x0, =0x6548206469766144
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-128]
# assign default
add x0, sp, #0
str x0, [fp, #-0x90]

# whatwha
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x61687774616877
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-160]
# assign default
add x0, sp, #0
str x0, [fp, #-0xb0]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
str x2, [fp, #-208]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #42
str x2, [fp, #-192]
# addition
ldr w0, [fp, #-192]
ldr w1, [fp, #-208]
add w0, w0, w1
str w0, [fp, #-224]

# assign (int) load
ldr w0, [fp, #-224]

# assign (int) store
str w0, [fp, #-240]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #17
sub x4, fp, #272
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #40
sub x4, fp, #256
str x2, [x4]
# subtraction
sub x4, fp, #256
ldr w0, [x4]
sub x4, fp, #272
ldr w1, [x4]
sub w0, w0, w1
sub x4, fp, #288
str w0, [x4]

# assign (int) load
sub x4, fp, #288
ldr w0, [x4]

# assign (int) store
sub x4, fp, #304
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
sub x4, fp, #336
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
sub x4, fp, #320
str x2, [x4]
# multiplication
sub x4, fp, #320
ldr w0, [x4]
sub x4, fp, #336
ldr w1, [x4]
mul w0, w0, w1
sub x4, fp, #352
str w0, [x4]

# assign (int) load
sub x4, fp, #352
ldr w0, [x4]

# assign (int) store
sub x4, fp, #368
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #400
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #25
sub x4, fp, #384
str x2, [x4]
# division
sub x4, fp, #384
ldr w0, [x4]
sub x4, fp, #400
ldr w1, [x4]
udiv w0, w0, w1
sub x4, fp, #416
str w0, [x4]

# assign (int) load
sub x4, fp, #416
ldr w0, [x4]

# assign (int) store
sub x4, fp, #432
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #480
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #464
str x2, [x4]
# multiplication
sub x4, fp, #464
ldr w0, [x4]
sub x4, fp, #480
ldr w1, [x4]
mul w0, w0, w1
sub x4, fp, #496
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #448
str x2, [x4]
# addition
sub x4, fp, #448
ldr w0, [x4]
sub x4, fp, #496
ldr w1, [x4]
add w0, w0, w1
sub x4, fp, #512
str w0, [x4]

# assign (int) load
sub x4, fp, #512
ldr w0, [x4]

# assign (int) store
sub x4, fp, #528
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #592
str x2, [x4]

 # compound (0x600001b8bc00) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #560
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #544
str x2, [x4]
# addition
sub x4, fp, #544
ldr w0, [x4]
sub x4, fp, #560
ldr w1, [x4]
add w0, w0, w1
sub x4, fp, #576
str w0, [x4]
# multiplication
sub x4, fp, #576
ldr w0, [x4]
sub x4, fp, #592
ldr w1, [x4]
mul w0, w0, w1
sub x4, fp, #608
str w0, [x4]

# assign (int) load
sub x4, fp, #608
ldr w0, [x4]

# assign (int) store
sub x4, fp, #624
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #672
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #656
str x2, [x4]
# division
sub x4, fp, #656
ldr w0, [x4]
sub x4, fp, #672
ldr w1, [x4]
udiv w0, w0, w1
sub x4, fp, #688
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
sub x4, fp, #640
str x2, [x4]
# subtraction
sub x4, fp, #640
ldr w0, [x4]
sub x4, fp, #688
ldr w1, [x4]
sub w0, w0, w1
sub x4, fp, #704
str w0, [x4]

# assign (int) load
sub x4, fp, #704
ldr w0, [x4]

# assign (int) store
sub x4, fp, #720
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #784
str x2, [x4]

 # compound (0x600001b8bf00) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #752
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
sub x4, fp, #736
str x2, [x4]
# subtraction
sub x4, fp, #736
ldr w0, [x4]
sub x4, fp, #752
ldr w1, [x4]
sub w0, w0, w1
sub x4, fp, #768
str w0, [x4]
# division
sub x4, fp, #768
ldr w0, [x4]
sub x4, fp, #784
ldr w1, [x4]
udiv w0, w0, w1
sub x4, fp, #800
str w0, [x4]

# assign (int) load
sub x4, fp, #800
ldr w0, [x4]

# assign (int) store
sub x4, fp, #816
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #912
str x2, [x4]

 # compound (0x600001b8c0c0) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #880
str x2, [x4]

 # compound (0x600001b8c180) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #848
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #832
str x2, [x4]
# addition
sub x4, fp, #832
ldr w0, [x4]
sub x4, fp, #848
ldr w1, [x4]
add w0, w0, w1
sub x4, fp, #864
str w0, [x4]
# multiplication
sub x4, fp, #864
ldr w0, [x4]
sub x4, fp, #880
ldr w1, [x4]
mul w0, w0, w1
sub x4, fp, #896
str w0, [x4]
# addition
sub x4, fp, #896
ldr w0, [x4]
sub x4, fp, #912
ldr w1, [x4]
add w0, w0, w1
sub x4, fp, #928
str w0, [x4]

# assign (int) load
sub x4, fp, #928
ldr w0, [x4]

# assign (int) store
sub x4, fp, #944
str w0, [x4]

# add 42+10=
sub sp, sp, #16
ldr x0, =0x3d30
str x0, [sp, #8]
ldr x0, =0x312b323420646461
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #960
str x0, [x4]

# variable (add)
ldr x0, [fp, #-240]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #960
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-240]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# sub 40-17=
sub sp, sp, #16
ldr x0, =0x3d37
str x0, [sp, #8]
ldr x0, =0x312d303420627573
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #992
str x0, [x4]

# variable (sub)
sub x4, fp, #304
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #992
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #304
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# mul 10*10=
sub sp, sp, #16
ldr x0, =0x3d30
str x0, [sp, #8]
ldr x0, =0x312a3031206c756d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1024
str x0, [x4]

# variable (mul)
sub x4, fp, #368
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1024
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #368
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# div 25/5=
sub sp, sp, #16
ldr x0, =0x3d
str x0, [sp, #8]
ldr x0, =0x352f353220766964
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1056
str x0, [x4]

# variable (div)
sub x4, fp, #432
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1056
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #432
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 2+3*4 (no paren)=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d
str x0, [sp, #16]
ldr x0, =0x296e65726170206f
str x0, [sp, #8]
ldr x0, =0x6e2820342a332b32
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1088
str x0, [x4]

# variable (mixed1)
sub x4, fp, #528
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1088
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #528
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# (2+3)*4 (paren)=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d296e6572617028
str x0, [sp, #8]
ldr x0, =0x20342a29332b3228
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1120
str x0, [x4]

# variable (mixed2)
sub x4, fp, #624
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1120
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #624
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 20-4/2 (no paren)=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d29
str x0, [sp, #16]
ldr x0, =0x6e65726170206f6e
str x0, [sp, #8]
ldr x0, =0x2820322f342d3032
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1152
str x0, [x4]

# variable (mixed3)
sub x4, fp, #720
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1152
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #720
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# (20-4)/2 (paren)=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d
str x0, [sp, #16]
ldr x0, =0x296e657261702820
str x0, [sp, #8]
ldr x0, =0x322f29342d303228
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1184
str x0, [x4]

# variable (mixed4)
sub x4, fp, #816
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1184
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #816
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# ((1+2)*3)+4=
sub sp, sp, #16
ldr x0, =0x3d342b29
str x0, [sp, #8]
ldr x0, =0x332a29322b312828
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1216
str x0, [x4]

# variable (nested)
sub x4, fp, #944
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1216
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #944
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# variable (what)
ldr x0, [fp, #-176]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-176]
bl HelloWorld

# store return value
str x0, [sp, #-16]!

# variable (name)
ldr x0, [fp, #-112]
str x0, [sp, #-16]!

# variable (name2)
ldr x0, [fp, #-144]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-112]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-144]
bl HelloWorld

# store return value
str x0, [sp, #-16]!

# Hello 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x206f6c6c6548
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1296
str x0, [x4]

# variable (name2)
ldr x0, [fp, #-144]
str x0, [sp, #-16]!

# ! 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x0a21
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1328
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1296
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-144]
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #1328
ldr x0, [x4]
bl HelloWorld

# store return value
str x0, [sp, #-16]!

# variable (name)
ldr x0, [fp, #-112]
str x0, [sp, #-16]!

# load arg 0 into x0
ldr x0, [fp, #-112]

# call function print
bl print

# store return value
str x0, [sp, #-16]!

# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1360
str x2, [x4]

b return_statement
# assign default
add x0, sp, #0
str x0, [fp, #-0x30]

# variable (int)
ldr x0, [fp, #0]
str x0, [sp, #-16]!
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

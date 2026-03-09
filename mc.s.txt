.text
.globl _start
_start:
bl main
mov x0, #0
mov x16, #1
svc #0

 # compound (0x600000a356e0) 
# start of "print"
.globl print
print:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #80

sub sp, sp, #96

# load parameter test from x0
str x0, [fp, #-48]

 # compound (0x600000a35860) 

# variable (test)
ldr x0, [fp, #-48]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-48]
bl HelloWorld

# store return value
str x0, [sp, #-16]!

mov w0, #30
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-16]
# start of "stringReturn"
.globl stringReturn
stringReturn:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #96

sub sp, sp, #112

# load parameter pushed from x0
str x0, [fp, #-48]

 # compound (0x600000a35aa0) 

# Taken string : 
sub sp, sp, #16
ldr x0, =0x203a20676e6972
str x0, [sp, #8]
ldr x0, =0x7473206e656b6154
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-64]

# variable (pushed)
ldr x0, [fp, #-48]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-64]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-48]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# Done
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x656e6f44
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-96]

b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-32]
# start of "testStringOps"
.globl testStringOps
testStringOps:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #896

sub sp, sp, #912

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600000a35c80) 

# --- String ops tests ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d2073747365
str x0, [sp, #16]
ldr x0, =0x742073706f20676e
str x0, [sp, #8]
ldr x0, =0x69727453202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-64]

# HelloWorld arg 0
ldr x0, [fp, #-64]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# Hello
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x6f6c6c6548
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-80]

# assign str load
ldr x0, [fp, #-80]

# assign str store
str x0, [fp, #-96]

# World
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x646c726f57
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-112]

# assign str load
ldr x0, [fp, #-112]

# assign str store
str x0, [fp, #-128]

# variable (a)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!

# variable (b)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!

# BigWord left
ldr x0, [fp, #-96]
str x0, [sp, #-16]!

# BigWord right
ldr x0, [fp, #-128]

# BigWord
mov x1, x0
ldr x0, [sp], #16
bl BigWord
str x0, [sp, #-16]!

# assign str binop (pop from stack)
ldr x0, [sp], #16

# assign binop store (str)
str x0, [fp, #-192]

# concat a+b: 
sub sp, sp, #16
ldr x0, =0x203a622b
str x0, [sp, #8]
ldr x0, =0x61207461636e6f63
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-208]

# variable (c)
ldr x0, [fp, #-192]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-208]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-192]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# hello
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x6f6c6c6568
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-240]

# assign str load
ldr x0, [fp, #-240]

# assign str store
sub x4, fp, #256
str x0, [x4]

# variable (s)
sub x4, fp, #256
ldr x0, [x4]
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #288
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #304
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #256
ldr x0, [x4]

# load arg 1 into x1
sub x4, fp, #288
ldr x1, [x4]

# load arg 2 into x2
sub x4, fp, #304
ldr x2, [x4]

# call function SmolString
bl SmolString

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #320
str x0, [x4]
add sp, sp, #16

# s[1:4]: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a5d343a315b73
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #336
str x0, [x4]

# variable (sub)
sub x4, fp, #320
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #336
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #320
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #384
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #400
str x2, [x4]

# load string for SmolString
sub x4, fp, #256
ldr x0, [x4]
bl rt_null_str_check
str x0, [sp, #-16]!

# load start
sub x4, fp, #384
ldr w0, [x4]
str w0, [sp, #-16]!

# load end
sub x4, fp, #400
ldr w0, [x4]

# SmolString (slice)
mov w2, w0
ldr w1, [sp], #16
ldr x0, [sp], #16
bl SmolString
str x0, [sp, #-16]!

# assign str (pop from stack)
ldr x0, [sp], #16

# assign str store
sub x4, fp, #432
str x0, [x4]

# s[one to four] 
sub sp, sp, #16
ldr x0, =0x205d72756f6620
str x0, [sp, #8]
ldr x0, =0x6f7420656e6f5b73
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #448
str x0, [x4]

# variable (subsi)
sub x4, fp, #432
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #448
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #432
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #480
str x2, [x4]

# load string for CharAt
sub x4, fp, #256
ldr x0, [x4]
bl rt_null_str_check
str x0, [sp, #-16]!

# load index for CharAt
sub x4, fp, #480
ldr w0, [x4]

# CharAt
mov w1, w0
ldr x0, [sp], #16
bl CharAt
str x0, [sp, #-16]!

# assign str (pop from stack)
ldr x0, [sp], #16

# assign str store
sub x4, fp, #512
str x0, [x4]

# s[0]: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a5d305b73
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #528
str x0, [x4]

# variable (ch)
sub x4, fp, #512
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #528
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #512
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# foo bar baz
sub sp, sp, #16
ldr x0, =0x7a6162
str x0, [sp, #8]
ldr x0, =0x20726162206f6f66
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #560
str x0, [x4]

# assign str load
sub x4, fp, #560
ldr x0, [x4]

# assign str store
sub x4, fp, #576
str x0, [x4]

# variable (t)
sub x4, fp, #576
ldr x0, [x4]
str x0, [sp, #-16]!

# bar
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x726162
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #608
str x0, [x4]

# qux
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x787571
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #624
str x0, [x4]

# load arg 0 into x0
sub x4, fp, #576
ldr x0, [x4]

# load arg 1 into x1
sub x4, fp, #608
ldr x1, [x4]

# load arg 2 into x2
sub x4, fp, #624
ldr x2, [x4]

# call function Change
bl Change

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #640
str x0, [x4]
add sp, sp, #16

# Change: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a65676e616843
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #656
str x0, [x4]

# variable (r)
sub x4, fp, #640
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #656
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #640
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

#   trim me  
sub sp, sp, #16
ldr x0, =0x202065
str x0, [sp, #8]
ldr x0, =0x6d206d6972742020
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #688
str x0, [x4]

# assign str load
sub x4, fp, #688
ldr x0, [x4]

# assign str store
sub x4, fp, #704
str x0, [x4]

# variable (sp)
sub x4, fp, #704
ldr x0, [x4]
str x0, [sp, #-16]!

# load arg 0 into x0
sub x4, fp, #704
ldr x0, [x4]

# call function Clipper
bl Clipper

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #736
str x0, [x4]
add sp, sp, #16

# Clipper: 
sub sp, sp, #16
ldr x0, =0x20
str x0, [sp, #8]
ldr x0, =0x3a72657070696c43
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #752
str x0, [x4]

# variable (sm)
sub x4, fp, #736
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #752
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #736
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# a,b,c
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x632c622c61
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #784
str x0, [x4]

# assign str load
sub x4, fp, #784
ldr x0, [x4]

# assign str store
sub x4, fp, #800
str x0, [x4]

# variable (split)
sub x4, fp, #800
ldr x0, [x4]
str x0, [sp, #-16]!

# ,
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x2c
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #832
str x0, [x4]

# load arg 0 into x0
sub x4, fp, #800
ldr x0, [x4]

# load arg 1 into x1
sub x4, fp, #832
ldr x1, [x4]

# call function SmolStrings
bl SmolStrings

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #848
str x0, [x4]
add sp, sp, #16

# SmolStrings: 
sub sp, sp, #16
ldr x0, =0x203a73676e
str x0, [sp, #8]
ldr x0, =0x697274536c6f6d53
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #864
str x0, [x4]

# variable (parts)
sub x4, fp, #848
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #864
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #848
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-48]
# start of "testArrayOps"
.globl testArrayOps
testArrayOps:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #1744

sub sp, sp, #1760

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600000a36460) 

# --- Array tests ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d
str x0, [sp, #16]
ldr x0, =0x2073747365742079
str x0, [sp, #8]
ldr x0, =0x61727241202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-64]

# HelloWorld arg 0
ldr x0, [fp, #-64]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
str x2, [fp, #-80]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
str x2, [fp, #-96]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #30
str x2, [fp, #-112]

# assign array (base_slot=5, count=3)
mov w0, #5

# store array base slot
str w0, [fp, #-128]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
str x2, [fp, #-144]

# load array index for bounds check
ldr w0, [fp, #-144]

# array bounds check (size=3)
mov w1, #3
bl rt_array_bounds_check

# load array base_slot
ldr w0, [fp, #-128]
str w0, [sp, #-16]!

# load array index
ldr w0, [fp, #-144]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
str w0, [fp, #-176]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
str x2, [fp, #-192]

# load array index for bounds check
ldr w0, [fp, #-192]

# array bounds check (size=3)
mov w1, #3
bl rt_array_bounds_check

# load array base_slot
ldr w0, [fp, #-128]
str w0, [sp, #-16]!

# load array index
ldr w0, [fp, #-192]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
str w0, [fp, #-224]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
str x2, [fp, #-240]

# load array index for bounds check
ldr w0, [fp, #-240]

# array bounds check (size=3)
mov w1, #3
bl rt_array_bounds_check

# load array base_slot
ldr w0, [fp, #-128]
str w0, [sp, #-16]!

# load array index
ldr w0, [fp, #-240]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #272
str w0, [x4]

# arr[0]: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a5d305b727261
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #288
str x0, [x4]

# variable (a0)
ldr x0, [fp, #-176]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #288
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-176]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# arr[1]: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a5d315b727261
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #320
str x0, [x4]

# variable (a1)
ldr x0, [fp, #-224]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #320
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-224]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# arr[2]: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a5d325b727261
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #352
str x0, [x4]

# variable (a2)
sub x4, fp, #272
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #352
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #272
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #42
sub x4, fp, #384
str x2, [x4]

# assign array (base_slot=24, count=1)
mov w0, #24

# store array base slot
sub x4, fp, #400
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #416
str x2, [x4]

# load array index for bounds check
sub x4, fp, #416
ldr w0, [x4]

# array bounds check (size=1)
mov w1, #1
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #400
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #416
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #448
str w0, [x4]

# single[0]: 
sub sp, sp, #16
ldr x0, =0x203a5d
str x0, [sp, #8]
ldr x0, =0x305b656c676e6973
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #464
str x0, [x4]

# variable (s0)
sub x4, fp, #448
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #464
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #448
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #496
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #512
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #528
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #544
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #560
str x2, [x4]

# assign array (base_slot=31, count=5)
mov w0, #31

# store array base slot
sub x4, fp, #576
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #592
str x2, [x4]

# load array index for bounds check
sub x4, fp, #592
ldr w0, [x4]

# array bounds check (size=5)
mov w1, #5
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #576
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #592
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #624
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #640
str x2, [x4]

# load array index for bounds check
sub x4, fp, #640
ldr w0, [x4]

# array bounds check (size=5)
mov w1, #5
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #576
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #640
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #672
str w0, [x4]

# five[0]: 
sub sp, sp, #16
ldr x0, =0x20
str x0, [sp, #8]
ldr x0, =0x3a5d305b65766966
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #688
str x0, [x4]

# variable (f0)
sub x4, fp, #624
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #688
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

# five[4]: 
sub sp, sp, #16
ldr x0, =0x20
str x0, [sp, #8]
ldr x0, =0x3a5d345b65766966
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #720
str x0, [x4]

# variable (f4)
sub x4, fp, #672
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #720
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #672
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #752
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #768
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #784
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #800
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #816
str x2, [x4]

# assign array (base_slot=47, count=5)
mov w0, #47

# store array base slot
sub x4, fp, #832
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #848
str x2, [x4]

# load array index for bounds check
sub x4, fp, #848
ldr w0, [x4]

# array bounds check (size=5)
mov w1, #5
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #832
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #848
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #880
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #896
str x2, [x4]

# load array index for bounds check
sub x4, fp, #896
ldr w0, [x4]

# array bounds check (size=5)
mov w1, #5
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #832
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #896
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #928
str w0, [x4]

# zeros[0]: 
sub sp, sp, #16
ldr x0, =0x203a
str x0, [sp, #8]
ldr x0, =0x5d305b736f72657a
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #944
str x0, [x4]

# variable (z0)
sub x4, fp, #880
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #944
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #880
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# zeros[4]: 
sub sp, sp, #16
ldr x0, =0x203a
str x0, [sp, #8]
ldr x0, =0x5d345b736f72657a
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #976
str x0, [x4]

# variable (z4)
sub x4, fp, #928
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #976
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #928
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #1008
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #1024
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #1040
str x2, [x4]

# assign array (base_slot=63, count=3)
mov w0, #63

# store array base slot
sub x4, fp, #1056
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1072
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1072
ldr w0, [x4]

# array bounds check (size=3)
mov w1, #3
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1056
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1072
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1104
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #1120
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1120
ldr w0, [x4]

# array bounds check (size=3)
mov w1, #3
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1056
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1120
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1152
str w0, [x4]

# sevens[0]: 
sub sp, sp, #16
ldr x0, =0x203a5d
str x0, [sp, #8]
ldr x0, =0x305b736e65766573
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1168
str x0, [x4]

# variable (sv0)
sub x4, fp, #1104
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1168
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1104
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# sevens[2]: 
sub sp, sp, #16
ldr x0, =0x203a5d
str x0, [sp, #8]
ldr x0, =0x325b736e65766573
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1200
str x0, [x4]

# variable (sv2)
sub x4, fp, #1152
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1200
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1152
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1232
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1248
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1264
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #1280
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #1296
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #99
sub x4, fp, #1312
str x2, [x4]

# assign array (base_slot=77, count=6)
mov w0, #77

# store array base slot
sub x4, fp, #1328
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1344
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1344
ldr w0, [x4]

# array bounds check (size=6)
mov w1, #6
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1328
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1344
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1376
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #1392
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1392
ldr w0, [x4]

# array bounds check (size=6)
mov w1, #6
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1328
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1392
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1424
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #1440
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1440
ldr w0, [x4]

# array bounds check (size=6)
mov w1, #6
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1328
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1440
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1472
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #4
sub x4, fp, #1488
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1488
ldr w0, [x4]

# array bounds check (size=6)
mov w1, #6
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1328
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1488
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1520
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1536
str x2, [x4]

# load array index for bounds check
sub x4, fp, #1536
ldr w0, [x4]

# array bounds check (size=6)
mov w1, #6
bl rt_array_bounds_check

# load array base_slot
sub x4, fp, #1328
ldr w0, [x4]
str w0, [sp, #-16]!

# load array index
sub x4, fp, #1536
ldr w0, [x4]

# array element access
ldr w1, [sp], #16
add w1, w1, w0
mov w3, #16
mul w1, w1, w3
mov x3, fp
sub x3, x3, x1
ldr x0, [x3]
str x0, [sp, #-16]!

# assign int (pop from stack)
ldr x0, [sp], #16

# assign int from access
sub x4, fp, #1568
str w0, [x4]

# range[0] (expect 0): 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a293020
str x0, [sp, #16]
ldr x0, =0x7463657078652820
str x0, [sp, #8]
ldr x0, =0x5d305b65676e6172
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1584
str x0, [x4]

# variable (r0)
sub x4, fp, #1376
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1584
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1376
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# range[2] (expect 0): 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a293020
str x0, [sp, #16]
ldr x0, =0x7463657078652820
str x0, [sp, #8]
ldr x0, =0x5d325b65676e6172
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1616
str x0, [x4]

# variable (r2)
sub x4, fp, #1424
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1616
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1424
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# range[3] (expect 1): 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a293120
str x0, [sp, #16]
ldr x0, =0x7463657078652820
str x0, [sp, #8]
ldr x0, =0x5d335b65676e6172
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1648
str x0, [x4]

# variable (r3)
sub x4, fp, #1472
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1648
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1472
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# range[4] (expect 1): 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a293120
str x0, [sp, #16]
ldr x0, =0x7463657078652820
str x0, [sp, #8]
ldr x0, =0x5d345b65676e6172
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1680
str x0, [x4]

# variable (r4)
sub x4, fp, #1520
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1680
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1520
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# range[5] (expect 99): 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a29393920
str x0, [sp, #16]
ldr x0, =0x7463657078652820
str x0, [sp, #8]
ldr x0, =0x5d355b65676e6172
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1712
str x0, [x4]

# variable (r5)
sub x4, fp, #1568
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1712
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1568
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-64]
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #1520

sub sp, sp, #1536

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600000a36e80) 

# Hello
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x6f6c6c6548
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-64]

# assign str load
ldr x0, [fp, #-64]

# assign str store
str x0, [fp, #-80]

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

# assign str load
ldr x0, [fp, #-96]

# assign str store
str x0, [fp, #-112]

# David Hello
sub sp, sp, #16
ldr x0, =0x6f6c6c
str x0, [sp, #8]
ldr x0, =0x6548206469766144
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-128]

# assign str load
ldr x0, [fp, #-128]

# assign str store
str x0, [fp, #-144]

# whatwha
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x61687774616877
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-160]

# assign str load
ldr x0, [fp, #-160]

# assign str store
str x0, [fp, #-176]
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

# div-by-zero check
sub x4, fp, #400
ldr w0, [x4]
bl rt_div_zero_check
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

 # compound (0x600000a37540) 
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

# div-by-zero check
sub x4, fp, #672
ldr w0, [x4]
bl rt_div_zero_check
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

 # compound (0x600000a37840) 
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

# div-by-zero check
sub x4, fp, #784
ldr w0, [x4]
bl rt_div_zero_check
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

 # compound (0x600000a379c0) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #880
str x2, [x4]

 # compound (0x600000a37a80) 
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

# assign call store (int)
sub x4, fp, #1360
str w0, [x4]
add sp, sp, #16

# returned value: 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a65756c617620
str x0, [sp, #8]
ldr x0, =0x64656e7275746572
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1376
str x0, [x4]

# variable (returnVal)
sub x4, fp, #1360
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1376
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1360
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# hah
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x686168
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1408
str x0, [x4]

# load arg 0 into x0
sub x4, fp, #1408
ldr x0, [x4]

# call function stringReturn
bl stringReturn

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #1424
str x0, [x4]
add sp, sp, #16

# variable (returnStr)
sub x4, fp, #1424
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1424
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1456
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #1456
ldr x0, [x4]

# call function testStringOps
bl testStringOps

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #1472
str w0, [x4]
add sp, sp, #16
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1488
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #1488
ldr x0, [x4]

# call function testArrayOps
bl testArrayOps

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #1504
str w0, [x4]
add sp, sp, #16

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-80]
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

.section __DATA,__bss
.p2align 4
_str_buf: .space 4096
_str_buf0: .space 512
_str_buf1: .space 512
_str_buf2: .space 512
_str_buf3: .space 512
_str_buf4: .space 512
_str_buf5: .space 512

.text
BigWord:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    mov x19, x0
    mov x20, x1
    bl strlen
    mov x21, x0
    mov x0, x20
    bl strlen
    add x2, x21, x0
    add x2, x2, #1
    adrp x3, _str_buf0@PAGE
    add x3, x3, _str_buf0@PAGEOFF
    mov x0, x3
    mov x1, x19
    mov x4, #0
BigWord_copy1:
    cmp x4, x21
    b.ge BigWord_copy2
    ldrb w5, [x1, x4]
    strb w5, [x3, x4]
    add x4, x4, #1
    b BigWord_copy1
BigWord_copy2:
    mov x1, x20
    mov x4, #0
BigWord_copy2_loop:
    ldrb w5, [x1, x4]
    cbz w5, BigWord_done
    add x6, x3, x21
    strb w5, [x6, x4]
    add x4, x4, #1
    b BigWord_copy2_loop
BigWord_done:
    add x6, x3, x21
    strb wzr, [x6, x4]
    mov x0, x3
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

SmolString:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x6, x0
    mov w7, w1
    mov w8, w2
    sub w9, w8, w7
    adrp x3, _str_buf1@PAGE
    add x3, x3, _str_buf1@PAGEOFF
    mov x4, #0
SmolString_loop:
    cmp w4, w9
    b.ge SmolString_end
    add x5, x7, x4
    ldrb w10, [x6, x5]
    strb w10, [x3, x4]
    add w4, w4, #1
    b SmolString_loop
SmolString_end:
    strb wzr, [x3, x4]
    mov x0, x3
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

CharAt:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    adrp x3, _str_buf2@PAGE
    add x3, x3, _str_buf2@PAGEOFF
    ldrb w4, [x0, x1]
    strb w4, [x3]
    strb wzr, [x3, #1]
    mov x0, x3
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

Change:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    stp x23, x24, [sp, #-16]!
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    mov x19, x0
    mov x20, x1
    mov x21, x2
    bl strlen
    mov x22, x0
    mov x0, x20
    bl strlen
    mov x23, x0
    mov x0, x21
    bl strlen
    mov x24, x0
    adrp x3, _str_buf3@PAGE
    add x3, x3, _str_buf3@PAGEOFF
    mov x4, #0
    mov x5, #0
Change_find:
    cmp x5, x22
    b.ge Change_nomatch
    mov x6, #0
Change_cmp:
    cmp x6, x23
    b.ge Change_found
    add x7, x19, x5
    add x7, x7, x6
    ldrb w8, [x7]
    ldrb w9, [x20, x6]
    cmp w8, w9
    b.ne Change_next
    add x6, x6, #1
    b Change_cmp
Change_next:
    add x5, x5, #1
    b Change_find
Change_found:
    mov x6, #0
Change_copy_before:
    cmp x6, x5
    b.ge Change_copy_new
    ldrb w8, [x19, x6]
    strb w8, [x3, x4]
    add x6, x6, #1
    add x4, x4, #1
    b Change_copy_before
Change_copy_new:
    mov x6, #0
Change_copy_new_loop:
    ldrb w8, [x21, x6]
    cbz w8, Change_copy_after
    strb w8, [x3, x4]
    add x6, x6, #1
    add x4, x4, #1
    b Change_copy_new_loop
Change_copy_after:
    add x5, x5, x23
Change_copy_after_loop:
    cmp x5, x22
    b.ge Change_done
    ldrb w8, [x19, x5]
    cbz w8, Change_done
    strb w8, [x3, x4]
    add x5, x5, #1
    add x4, x4, #1
    b Change_copy_after_loop
Change_done:
    strb wzr, [x3, x4]
    mov x0, x3
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
    ldp x23, x24, [sp], #16
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret
Change_nomatch:
    mov x0, x19
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
    ldp x23, x24, [sp], #16
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

Clipper:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    mov x7, x0
    bl strlen
    mov x6, x0
    mov x8, #0
Clipper_lead:
    cmp x8, x6
    b.ge Clipper_trail
    ldrb w9, [x7, x8]
    cmp w9, #32
    b.lo Clipper_trail
    cmp w9, #32
    b.gt Clipper_trail
    add x8, x8, #1
    b Clipper_lead
Clipper_trail:
    cmp x6, #0
    b.eq Clipper_empty
    sub x10, x6, #1
Clipper_trail_loop:
    cmp x10, x8
    b.lo Clipper_copy
    ldrb w9, [x7, x10]
    cmp w9, #32
    b.lo Clipper_copy
    cmp w9, #32
    b.gt Clipper_copy
    sub x10, x10, #1
    b Clipper_trail_loop
Clipper_copy:
    adrp x3, _str_buf4@PAGE
    add x3, x3, _str_buf4@PAGEOFF
    mov x4, #0
Clipper_copy_loop:
    add x5, x8, x4
    cmp x5, x10
    b.gt Clipper_copy_done
    ldrb w9, [x7, x5]
    strb w9, [x3, x4]
    add x4, x4, #1
    b Clipper_copy_loop
Clipper_copy_done:
    strb wzr, [x3, x4]
    mov x0, x3
Clipper_ret:
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret
Clipper_empty:
    mov x0, x7
    b Clipper_ret

SmolStrings:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    stp x19, x20, [sp, #-16]!
    stp x21, x22, [sp, #-16]!
    mov x19, x0
    mov x20, x1
    bl strlen
    mov x21, x0
    mov x0, x20
    bl strlen
    mov x22, x0
    adrp x3, _str_buf5@PAGE
    add x3, x3, _str_buf5@PAGEOFF
    mov x4, #0
    mov x5, #0
    mov x6, #0
SmolStrings_loop:
    cmp x5, x21
    b.ge SmolStrings_done
    mov x7, #0
SmolStrings_cmp:
    cmp x7, x22
    b.ge SmolStrings_delim
    add x8, x5, x7
    cmp x8, x21
    b.ge SmolStrings_part
    ldrb w9, [x19, x8]
    ldrb w10, [x20, x7]
    cmp w9, w10
    b.ne SmolStrings_part
    add x7, x7, #1
    b SmolStrings_cmp
SmolStrings_delim:
    cmp x4, #0
    b.eq SmolStrings_skip_sep
    mov w9, #1
    strb w9, [x3, x4]
    add x4, x4, #1
SmolStrings_skip_sep:
    add x5, x5, x22
    b SmolStrings_loop
SmolStrings_part:
    cmp x5, x21
    b.ge SmolStrings_done
    ldrb w9, [x19, x5]
    cbz w9, SmolStrings_done
    mov x7, #0
SmolStrings_cmp2:
    cmp x7, x22
    b.ge SmolStrings_delim
    add x8, x5, x7
    cmp x8, x21
    b.ge SmolStrings_part_end
    ldrb w10, [x19, x8]
    ldrb w11, [x20, x7]
    cmp w10, w11
    b.ne SmolStrings_part_end
    add x7, x7, #1
    b SmolStrings_cmp2
SmolStrings_part_end:
    strb w9, [x3, x4]
    add x4, x4, #1
    add x5, x5, #1
    b SmolStrings_part
SmolStrings_done:
    strb wzr, [x3, x4]
    mov x0, x3
    ldp x21, x22, [sp], #16
    ldp x19, x20, [sp], #16
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

# ---- Runtime error handlers ----

_rt_err_oob:
.asciz "Runtime Error: Array index out of bounds\n"
.p2align 2
_rt_err_null_str:
.asciz "Runtime Error: Null string access\n"
.p2align 2
_rt_err_div_zero:
.asciz "Runtime Error: Division by zero\n"
.p2align 2

rt_error_exit:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    bl strlen
    mov x2, x0
    mov x1, x6
    mov x0, #2
    mov x16, #4
    svc #0
    mov x0, #1
    mov x16, #1
    svc #0

rt_array_bounds_check:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    cmp w0, #0
    b.lt rt_array_oob
    cmp w0, w1
    b.ge rt_array_oob
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret
rt_array_oob:
    adrp x6, _rt_err_oob@PAGE
    add x6, x6, _rt_err_oob@PAGEOFF
    mov x0, x6
    b rt_error_exit

rt_null_str_check:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    cbnz x0, rt_null_str_ok
    adrp x6, _rt_err_null_str@PAGE
    add x6, x6, _rt_err_null_str@PAGEOFF
    mov x0, x6
    b rt_error_exit
rt_null_str_ok:
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

rt_div_zero_check:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    cbnz w0, rt_div_zero_ok
    adrp x6, _rt_err_div_zero@PAGE
    add x6, x6, _rt_err_div_zero@PAGEOFF
    mov x0, x6
    b rt_error_exit
rt_div_zero_ok:
    mov sp, x29
    ldp x29, x30, [sp], #16
    ret

.text
.globl _start
_start:
bl main
mov x0, #0
mov x16, #1
svc #0

 # compound (0x600001f3b5a0) 
# start of "print"
.globl print
print:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #80

sub sp, sp, #96

# load parameter test from x0
str x0, [fp, #-48]

 # compound (0x600001f3b720) 

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

 # compound (0x600001f3b960) 

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
sub sp, sp, #1184

sub sp, sp, #1200

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001f3bb40) 

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

# --- String if-checks ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d20736b6365
str x0, [sp, #16]
ldr x0, =0x68632d666920676e
str x0, [sp, #8]
ldr x0, =0x69727453202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #896
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #896
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #912
str x2, [x4]

# assign (int) load
sub x4, fp, #912
ldr w0, [x4]

# assign (int) store
sub x4, fp, #928
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #960
str x2, [x4]

# variable (slen)
sub x4, fp, #928
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #928
ldr w0, [x4]
sub x4, fp, #960
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #976
str w0, [x4]

# if condition
sub x4, fp, #976
ldr w0, [x4]
cbz w0, _else_0

# PASS: string length is 5
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3520736920687467
str x0, [sp, #16]
ldr x0, =0x6e656c20676e6972
str x0, [sp, #8]
ldr x0, =0x7473203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #992
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #992
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_0
_else_0:

# FAIL: string length
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x687467
str x0, [sp, #16]
ldr x0, =0x6e656c20676e6972
str x0, [sp, #8]
ldr x0, =0x7473203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1008
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1008
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_0:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1024
str x2, [x4]

# assign (int) load
sub x4, fp, #1024
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1040
str w0, [x4]

# variable (idx)
sub x4, fp, #1040
ldr x0, [x4]
str x0, [sp, #-16]!

# load string for CharAt
sub x4, fp, #256
ldr x0, [x4]
bl rt_null_str_check
str x0, [sp, #-16]!

# load index for CharAt
sub x4, fp, #1040
ldr w0, [x4]

# CharAt
mov w1, w0
ldr x0, [sp], #16
bl CharAt
str x0, [sp, #-16]!

# assign str (pop from stack)
ldr x0, [sp], #16

# assign str store
sub x4, fp, #1088
str x0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #1120
str x2, [x4]

# variable (idx)
sub x4, fp, #1040
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1040
ldr w0, [x4]
sub x4, fp, #1120
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1136
str w0, [x4]

# if condition
sub x4, fp, #1136
ldr w0, [x4]
cbz w0, _else_2

# PASS: index 0 access
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x73736563
str x0, [sp, #16]
ldr x0, =0x6361203020786564
str x0, [sp, #8]
ldr x0, =0x6e69203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1152
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1152
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_2
_else_2:

# FAIL: index 0 access
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x73736563
str x0, [sp, #16]
ldr x0, =0x6361203020786564
str x0, [sp, #8]
ldr x0, =0x6e69203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1168
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1168
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_2:

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
sub sp, sp, #2560

sub sp, sp, #2576

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001f34720) 

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

# --- Array if-checks ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d20736b63
str x0, [sp, #16]
ldr x0, =0x6568632d66692079
str x0, [sp, #8]
ldr x0, =0x61727241202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1744
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1744
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
sub x4, fp, #1776
str x2, [x4]

# variable (a0)
ldr x0, [fp, #-176]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #176
ldr w0, [x4]
sub x4, fp, #1776
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1792
str w0, [x4]

# if condition
sub x4, fp, #1792
ldr w0, [x4]
cbz w0, _else_4

# PASS: arr[0]==10
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x30313d3d5d305b72
str x0, [sp, #8]
ldr x0, =0x7261203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1808
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1808
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_4
_else_4:

# FAIL: arr[0]!=10
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x30313d215d305b72
str x0, [sp, #8]
ldr x0, =0x7261203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1824
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1824
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_4:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #30
sub x4, fp, #1904
str x2, [x4]

# variable (a2)
sub x4, fp, #272
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #272
ldr w0, [x4]
sub x4, fp, #1904
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1920
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
sub x4, fp, #1856
str x2, [x4]

# variable (a1)
ldr x0, [fp, #-224]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #224
ldr w0, [x4]
sub x4, fp, #1856
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1872
str w0, [x4]
# logical and
sub x4, fp, #1872
ldr w0, [x4]
sub x4, fp, #1920
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #1936
str w0, [x4]

# if condition
sub x4, fp, #1936
ldr w0, [x4]
cbz w0, _else_6

# PASS: arr[1]==20 and arr[2]==30
sub sp, sp, #32
ldr x0, =0x30333d3d5d325b
str x0, [sp, #24]
ldr x0, =0x72726120646e6120
str x0, [sp, #16]
ldr x0, =0x30323d3d5d315b72
str x0, [sp, #8]
ldr x0, =0x7261203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1952
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1952
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_6
_else_6:

# FAIL: arr values
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x7365756c61762072
str x0, [sp, #8]
ldr x0, =0x7261203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1968
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1968
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_6:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #42
sub x4, fp, #2000
str x2, [x4]

# variable (s0)
sub x4, fp, #448
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #448
ldr w0, [x4]
sub x4, fp, #2000
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2016
str w0, [x4]

# if condition
sub x4, fp, #2016
ldr w0, [x4]
cbz w0, _else_8

# PASS: single[0]==42
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x32343d
str x0, [sp, #16]
ldr x0, =0x3d5d305b656c676e
str x0, [sp, #8]
ldr x0, =0x6973203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2032
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2032
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_8
_else_8:

# FAIL: single[0]
sub sp, sp, #16
ldr x0, =0x5d305b656c676e
str x0, [sp, #8]
ldr x0, =0x6973203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2048
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2048
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_8:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2128
str x2, [x4]

# variable (z4)
sub x4, fp, #928
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #928
ldr w0, [x4]
sub x4, fp, #2128
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2144
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2080
str x2, [x4]

# variable (z0)
sub x4, fp, #880
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #880
ldr w0, [x4]
sub x4, fp, #2080
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2096
str w0, [x4]
# logical and
sub x4, fp, #2096
ldr w0, [x4]
sub x4, fp, #2144
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2160
str w0, [x4]

# if condition
sub x4, fp, #2160
ldr w0, [x4]
cbz w0, _else_10

# PASS: zeros all 0
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x30
str x0, [sp, #16]
ldr x0, =0x206c6c6120736f72
str x0, [sp, #8]
ldr x0, =0x657a203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2176
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2176
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_10
_else_10:

# FAIL: zeros
sub sp, sp, #16
ldr x0, =0x736f72
str x0, [sp, #8]
ldr x0, =0x657a203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2192
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2192
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_10:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #2272
str x2, [x4]

# variable (sv2)
sub x4, fp, #1152
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1152
ldr w0, [x4]
sub x4, fp, #2272
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2288
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #2224
str x2, [x4]

# variable (sv0)
sub x4, fp, #1104
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1104
ldr w0, [x4]
sub x4, fp, #2224
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2240
str w0, [x4]
# logical and
sub x4, fp, #2240
ldr w0, [x4]
sub x4, fp, #2288
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2304
str w0, [x4]

# if condition
sub x4, fp, #2304
ldr w0, [x4]
cbz w0, _else_12

# PASS: sevens all 7
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3720
str x0, [sp, #16]
ldr x0, =0x6c6c6120736e6576
str x0, [sp, #8]
ldr x0, =0x6573203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2320
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2320
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_12
_else_12:

# FAIL: sevens
sub sp, sp, #16
ldr x0, =0x736e6576
str x0, [sp, #8]
ldr x0, =0x6573203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2336
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2336
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_12:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #99
sub x4, fp, #2480
str x2, [x4]

# variable (r5)
sub x4, fp, #1568
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1568
ldr w0, [x4]
sub x4, fp, #2480
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2496
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #2416
str x2, [x4]

# variable (r3)
sub x4, fp, #1472
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1472
ldr w0, [x4]
sub x4, fp, #2416
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2432
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2368
str x2, [x4]

# variable (r0)
sub x4, fp, #1376
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1376
ldr w0, [x4]
sub x4, fp, #2368
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2384
str w0, [x4]
# logical and
sub x4, fp, #2384
ldr w0, [x4]
sub x4, fp, #2432
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2448
str w0, [x4]
# logical and
sub x4, fp, #2448
ldr w0, [x4]
sub x4, fp, #2496
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2512
str w0, [x4]

# if condition
sub x4, fp, #2512
ldr w0, [x4]
cbz w0, _else_14

# PASS: range values correct
sub sp, sp, #32
ldr x0, =0x7463
str x0, [sp, #24]
ldr x0, =0x6572726f63207365
str x0, [sp, #16]
ldr x0, =0x756c61762065676e
str x0, [sp, #8]
ldr x0, =0x6172203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2528
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2528
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_14
_else_14:

# FAIL: range values
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x7365
str x0, [sp, #16]
ldr x0, =0x756c61762065676e
str x0, [sp, #8]
ldr x0, =0x6172203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2544
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2544
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_14:

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-64]
# start of "testBoolAndIf"
.globl testBoolAndIf
testBoolAndIf:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #2640

sub sp, sp, #2656

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001f35c20) 

# --- Bool & If tests ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d20737473
str x0, [sp, #16]
ldr x0, =0x6574206649202620
str x0, [sp, #8]
ldr x0, =0x6c6f6f42202d2d2d
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
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
str w2, [fp, #-80]

# assign (int) load
ldr w0, [fp, #-80]

# assign (int) store
str w0, [fp, #-96]
# bool (Fake)
str x0, [sp, #-16]!
mov w2, #0
str w2, [fp, #-112]

# assign (int) load
ldr w0, [fp, #-112]

# assign (int) store
str w0, [fp, #-128]

# t = 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203d2074
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-144]

# variable (t)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-144]
bl HelloWorld

# HelloWorld arg 1
ldr w0, [fp, #-96]
cmp w0, #0
b.eq _btos_fake_0
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_0
_btos_fake_0:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_0:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# f = 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203d2066
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-176]

# variable (f)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-176]
bl HelloWorld

# HelloWorld arg 1
ldr w0, [fp, #-128]
cmp w0, #0
b.eq _btos_fake_1
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_1
_btos_fake_1:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_1:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# variable (f)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!

# logical not load
ldr w0, [fp, #-128]
cmp w0, #0
cset w0, eq

# logical not store
str w0, [fp, #-224]

# assign (int) load
ldr w0, [fp, #-224]

# assign (int) store
str w0, [fp, #-240]

# not Fake = 
sub sp, sp, #16
ldr x0, =0x203d20
str x0, [sp, #8]
ldr x0, =0x656b614620746f6e
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #256
str x0, [x4]

# variable (r)
ldr x0, [fp, #-240]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #256
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr w0, [fp, #-240]
cmp w0, #0
b.eq _btos_fake_2
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_2
_btos_fake_2:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_2:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# variable (f)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!

# variable (t)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!
# logical and
sub x4, fp, #96
ldr w0, [x4]
sub x4, fp, #128
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #320
str w0, [x4]

# assign (int) load
sub x4, fp, #320
ldr w0, [x4]

# assign (int) store
sub x4, fp, #336
str w0, [x4]

# variable (f)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!

# variable (t)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!
# logical or
sub x4, fp, #96
ldr w0, [x4]
sub x4, fp, #128
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
orr w0, w0, w1
sub x4, fp, #384
str w0, [x4]

# assign (int) load
sub x4, fp, #384
ldr w0, [x4]

# assign (int) store
sub x4, fp, #400
str w0, [x4]

# Real and Fake = 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203d20656b614620
str x0, [sp, #8]
ldr x0, =0x646e61206c616552
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #416
str x0, [x4]

# variable (a)
sub x4, fp, #336
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #416
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #336
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_3
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_3
_btos_fake_3:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_3:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# Real or Fake = 
sub sp, sp, #16
ldr x0, =0x203d20656b6146
str x0, [sp, #8]
ldr x0, =0x20726f206c616552
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #448
str x0, [x4]

# variable (o)
sub x4, fp, #400
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #448
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #400
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_4
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_4
_btos_fake_4:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_4:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #496
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #480
str x2, [x4]
# bitwise and
sub x4, fp, #480
ldr w0, [x4]
sub x4, fp, #496
ldr w1, [x4]
and w0, w0, w1
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
mov x2, #3
sub x4, fp, #560
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #544
str x2, [x4]
# bitwise or
sub x4, fp, #544
ldr w0, [x4]
sub x4, fp, #560
ldr w1, [x4]
orr w0, w0, w1
sub x4, fp, #576
str w0, [x4]

# assign (int) load
sub x4, fp, #576
ldr w0, [x4]

# assign (int) store
sub x4, fp, #592
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #608
str x2, [x4]

# bitwise not load
sub x4, fp, #608
ldr w0, [x4]
mvn w0, w0

# bitwise not store
sub x4, fp, #624
str w0, [x4]

# assign (int) load
sub x4, fp, #624
ldr w0, [x4]

# assign (int) store
sub x4, fp, #640
str w0, [x4]

# 5 & 3 = 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203d203320262035
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #656
str x0, [x4]

# variable (ba)
sub x4, fp, #528
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #656
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

# 5 | 3 = 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203d2033207c2035
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #688
str x0, [x4]

# variable (bo)
sub x4, fp, #592
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #688
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #592
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# ~0 = 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203d20307e
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #720
str x0, [x4]

# variable (bn)
sub x4, fp, #640
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #720
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #640
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
mov x2, #5
sub x4, fp, #768
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #752
str x2, [x4]
# comparison
sub x4, fp, #752
ldr w0, [x4]
sub x4, fp, #768
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #784
str w0, [x4]

# assign (int) load
sub x4, fp, #784
ldr w0, [x4]

# assign (int) store
sub x4, fp, #800
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #832
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #816
str x2, [x4]
# comparison
sub x4, fp, #816
ldr w0, [x4]
sub x4, fp, #832
ldr w1, [x4]
cmp w0, w1
cset w0, ne
sub x4, fp, #848
str w0, [x4]

# assign (int) load
sub x4, fp, #848
ldr w0, [x4]

# assign (int) store
sub x4, fp, #864
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #896
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #880
str x2, [x4]
# comparison
sub x4, fp, #880
ldr w0, [x4]
sub x4, fp, #896
ldr w1, [x4]
cmp w0, w1
cset w0, lt
sub x4, fp, #912
str w0, [x4]

# assign (int) load
sub x4, fp, #912
ldr w0, [x4]

# assign (int) store
sub x4, fp, #928
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #960
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #944
str x2, [x4]
# comparison
sub x4, fp, #944
ldr w0, [x4]
sub x4, fp, #960
ldr w1, [x4]
cmp w0, w1
cset w0, gt
sub x4, fp, #976
str w0, [x4]

# assign (int) load
sub x4, fp, #976
ldr w0, [x4]

# assign (int) store
sub x4, fp, #992
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1024
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1008
str x2, [x4]
# comparison
sub x4, fp, #1008
ldr w0, [x4]
sub x4, fp, #1024
ldr w1, [x4]
cmp w0, w1
cset w0, le
sub x4, fp, #1040
str w0, [x4]

# assign (int) load
sub x4, fp, #1040
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1056
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1088
str x2, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #1072
str x2, [x4]
# comparison
sub x4, fp, #1072
ldr w0, [x4]
sub x4, fp, #1088
ldr w1, [x4]
cmp w0, w1
cset w0, ge
sub x4, fp, #1104
str w0, [x4]

# assign (int) load
sub x4, fp, #1104
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1120
str w0, [x4]

# 5==5: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a353d3d35
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1136
str x0, [x4]

# variable (eq)
sub x4, fp, #800
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1136
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #800
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_5
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_5
_btos_fake_5:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_5:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 5!=3: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a333d2135
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1168
str x0, [x4]

# variable (ne)
sub x4, fp, #864
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1168
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #864
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_6
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_6
_btos_fake_6:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_6:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 3<5: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a353c33
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1200
str x0, [x4]

# variable (lt)
sub x4, fp, #928
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1200
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #928
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_7
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_7
_btos_fake_7:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_7:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 5>3: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a333e35
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1232
str x0, [x4]

# variable (gt)
sub x4, fp, #992
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1232
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #992
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_8
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_8
_btos_fake_8:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_8:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 5<=5: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a353d3c35
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1264
str x0, [x4]

# variable (le)
sub x4, fp, #1056
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1264
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1056
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_9
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_9
_btos_fake_9:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_9:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# 3>=5: 
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x203a353d3e33
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1296
str x0, [x4]

# variable (ge)
sub x4, fp, #1120
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1296
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1120
ldr w0, [x4]
cmp w0, #0
b.eq _btos_fake_10
sub sp, sp, #16
mov w1, #0x6552
movk w1, #0x6c61, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
b _btos_end_10
_btos_fake_10:
sub sp, sp, #16
mov w1, #0x6146
movk w1, #0x656b, lsl #16
str w1, [sp]
strb wzr, [sp, #4]
mov x1, sp
mov x2, #4
mov x0, #1
mov x16, #4
svc #0
add sp, sp, #16
_btos_end_10:

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #42
sub x4, fp, #1328
str x2, [x4]

# assign (int) load
sub x4, fp, #1328
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1344
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #42
sub x4, fp, #1376
str x2, [x4]

# variable (val)
sub x4, fp, #1344
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1344
ldr w0, [x4]
sub x4, fp, #1376
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1392
str w0, [x4]

# if condition
sub x4, fp, #1392
ldr w0, [x4]
cbz w0, _endif_16

# if: val is 42
sub sp, sp, #16
ldr x0, =0x3234207369
str x0, [sp, #8]
ldr x0, =0x206c6176203a6669
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1408
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1408
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_16
_endif_16:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #10
sub x4, fp, #1424
str x2, [x4]

# assign (int) load
sub x4, fp, #1424
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1440
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
sub x4, fp, #1472
str x2, [x4]

# variable (n)
sub x4, fp, #1440
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1440
ldr w0, [x4]
sub x4, fp, #1472
ldr w1, [x4]
cmp w0, w1
cset w0, gt
sub x4, fp, #1488
str w0, [x4]

# if condition
sub x4, fp, #1488
ldr w0, [x4]
cbz w0, _else_17

# if: n>20
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x30323e6e203a6669
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1504
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1504
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_17
_else_17:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1536
str x2, [x4]

# variable (n)
sub x4, fp, #1440
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1440
ldr w0, [x4]
sub x4, fp, #1536
ldr w1, [x4]
cmp w0, w1
cset w0, gt
sub x4, fp, #1552
str w0, [x4]

# if condition
sub x4, fp, #1552
ldr w0, [x4]
cbz w0, _else_18

# else if: n>5
sub sp, sp, #16
ldr x0, =0x353e6e20
str x0, [sp, #8]
ldr x0, =0x3a66692065736c65
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1568
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1568
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_18
_else_18:

# else: n<=5
sub sp, sp, #16
ldr x0, =0x353d
str x0, [sp, #8]
ldr x0, =0x3c6e203a65736c65
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1584
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1584
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_18:
_endif_17:
# bool (Fake)
str x0, [sp, #-16]!
mov w2, #0
sub x4, fp, #1600
str w2, [x4]

# if condition
sub x4, fp, #1600
ldr w0, [x4]
cbz w0, _else_20

# ERROR: should not print
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x746e6972702074
str x0, [sp, #16]
ldr x0, =0x6f6e20646c756f68
str x0, [sp, #8]
ldr x0, =0x73203a524f525245
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1616
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1616
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_20
_else_20:

# else: Fake branch skipped
sub sp, sp, #32
ldr x0, =0x64
str x0, [sp, #24]
ldr x0, =0x657070696b732068
str x0, [sp, #16]
ldr x0, =0x636e61726220656b
str x0, [sp, #8]
ldr x0, =0x6146203a65736c65
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1632
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1632
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_20:

# --- Bool if-checks ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d20736b
str x0, [sp, #16]
ldr x0, =0x636568632d666920
str x0, [sp, #8]
ldr x0, =0x6c6f6f42202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1648
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1648
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #1680
str w2, [x4]

# variable (t)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #96
ldr w0, [x4]
sub x4, fp, #1680
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1696
str w0, [x4]

# if condition
sub x4, fp, #1696
ldr w0, [x4]
cbz w0, _else_22

# PASS: t is Real
sub sp, sp, #16
ldr x0, =0x6c616552207369
str x0, [sp, #8]
ldr x0, =0x2074203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1712
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1712
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_22
_else_22:

# FAIL: t is not Real
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x6c6165
str x0, [sp, #16]
ldr x0, =0x5220746f6e207369
str x0, [sp, #8]
ldr x0, =0x2074203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1728
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1728
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_22:
# bool (Fake)
str x0, [sp, #-16]!
mov w2, #0
sub x4, fp, #1760
str w2, [x4]

# variable (f)
ldr x0, [fp, #-128]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #128
ldr w0, [x4]
sub x4, fp, #1760
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1776
str w0, [x4]

# if condition
sub x4, fp, #1776
ldr w0, [x4]
cbz w0, _else_24

# PASS: f is Fake
sub sp, sp, #16
ldr x0, =0x656b6146207369
str x0, [sp, #8]
ldr x0, =0x2066203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1792
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1792
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_24
_else_24:

# FAIL: f is not Fake
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x656b61
str x0, [sp, #16]
ldr x0, =0x4620746f6e207369
str x0, [sp, #8]
ldr x0, =0x2066203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1808
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1808
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_24:
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #1840
str w2, [x4]

# variable (r)
ldr x0, [fp, #-240]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #240
ldr w0, [x4]
sub x4, fp, #1840
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1856
str w0, [x4]

# if condition
sub x4, fp, #1856
ldr w0, [x4]
cbz w0, _else_26

# PASS: not Fake == Real
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x6c616552203d
str x0, [sp, #16]
ldr x0, =0x3d20656b61462074
str x0, [sp, #8]
ldr x0, =0x6f6e203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1872
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1872
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_26
_else_26:

# FAIL: not Fake != Real
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x6c616552203d
str x0, [sp, #16]
ldr x0, =0x2120656b61462074
str x0, [sp, #8]
ldr x0, =0x6f6e203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1888
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1888
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_26:
# bool (Fake)
str x0, [sp, #-16]!
mov w2, #0
sub x4, fp, #1920
str w2, [x4]

# variable (a)
sub x4, fp, #336
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #336
ldr w0, [x4]
sub x4, fp, #1920
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1936
str w0, [x4]

# if condition
sub x4, fp, #1936
ldr w0, [x4]
cbz w0, _else_28

# PASS: Real and Fake == Fake
sub sp, sp, #32
ldr x0, =0x656b61
str x0, [sp, #24]
ldr x0, =0x46203d3d20656b61
str x0, [sp, #16]
ldr x0, =0x4620646e61206c61
str x0, [sp, #8]
ldr x0, =0x6552203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1952
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1952
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_28
_else_28:

# FAIL: Real and Fake != Fake
sub sp, sp, #32
ldr x0, =0x656b61
str x0, [sp, #24]
ldr x0, =0x46203d2120656b61
str x0, [sp, #16]
ldr x0, =0x4620646e61206c61
str x0, [sp, #8]
ldr x0, =0x6552203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1968
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1968
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_28:
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2000
str w2, [x4]

# variable (o)
sub x4, fp, #400
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #400
ldr w0, [x4]
sub x4, fp, #2000
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2016
str w0, [x4]

# if condition
sub x4, fp, #2016
ldr w0, [x4]
cbz w0, _else_30

# PASS: Real or Fake == Real
sub sp, sp, #32
ldr x0, =0x6c61
str x0, [sp, #24]
ldr x0, =0x6552203d3d20656b
str x0, [sp, #16]
ldr x0, =0x614620726f206c61
str x0, [sp, #8]
ldr x0, =0x6552203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2032
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2032
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_30
_else_30:

# FAIL: Real or Fake != Real
sub sp, sp, #32
ldr x0, =0x6c61
str x0, [sp, #24]
ldr x0, =0x6552203d2120656b
str x0, [sp, #16]
ldr x0, =0x614620726f206c61
str x0, [sp, #8]
ldr x0, =0x6552203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2048
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2048
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_30:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #1
sub x4, fp, #2080
str x2, [x4]

# variable (ba)
sub x4, fp, #528
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #528
ldr w0, [x4]
sub x4, fp, #2080
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2096
str w0, [x4]

# if condition
sub x4, fp, #2096
ldr w0, [x4]
cbz w0, _else_32

# PASS: 5&3==1
sub sp, sp, #16
ldr x0, =0x313d3d33
str x0, [sp, #8]
ldr x0, =0x2635203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2112
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2112
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_32
_else_32:

# FAIL: 5&3
sub sp, sp, #16
ldr x0, =0x33
str x0, [sp, #8]
ldr x0, =0x2635203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2128
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2128
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_32:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #7
sub x4, fp, #2160
str x2, [x4]

# variable (bo)
sub x4, fp, #592
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #592
ldr w0, [x4]
sub x4, fp, #2160
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2176
str w0, [x4]

# if condition
sub x4, fp, #2176
ldr w0, [x4]
cbz w0, _else_34

# PASS: 5|3==7
sub sp, sp, #16
ldr x0, =0x373d3d33
str x0, [sp, #8]
ldr x0, =0x7c35203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2192
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2192
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_34
_else_34:

# FAIL: 5|3
sub sp, sp, #16
ldr x0, =0x33
str x0, [sp, #8]
ldr x0, =0x7c35203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2208
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2208
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_34:
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2480
str w2, [x4]

# variable (le)
sub x4, fp, #1056
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1056
ldr w0, [x4]
sub x4, fp, #2480
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2496
str w0, [x4]
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2416
str w2, [x4]

# variable (gt)
sub x4, fp, #992
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #992
ldr w0, [x4]
sub x4, fp, #2416
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2432
str w0, [x4]
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2352
str w2, [x4]

# variable (lt)
sub x4, fp, #928
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #928
ldr w0, [x4]
sub x4, fp, #2352
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2368
str w0, [x4]
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2288
str w2, [x4]

# variable (ne)
sub x4, fp, #864
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #864
ldr w0, [x4]
sub x4, fp, #2288
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2304
str w0, [x4]
# bool (Real)
str x0, [sp, #-16]!
mov w2, #1
sub x4, fp, #2240
str w2, [x4]

# variable (eq)
sub x4, fp, #800
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #800
ldr w0, [x4]
sub x4, fp, #2240
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2256
str w0, [x4]
# logical and
sub x4, fp, #2256
ldr w0, [x4]
sub x4, fp, #2304
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2320
str w0, [x4]
# logical and
sub x4, fp, #2320
ldr w0, [x4]
sub x4, fp, #2368
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2384
str w0, [x4]
# logical and
sub x4, fp, #2384
ldr w0, [x4]
sub x4, fp, #2432
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2448
str w0, [x4]
# logical and
sub x4, fp, #2448
ldr w0, [x4]
sub x4, fp, #2496
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #2512
str w0, [x4]

# if condition
sub x4, fp, #2512
ldr w0, [x4]
cbz w0, _else_36

# PASS: comparisons correct
sub sp, sp, #32
ldr x0, =0x74
str x0, [sp, #24]
ldr x0, =0x636572726f632073
str x0, [sp, #16]
ldr x0, =0x6e6f73697261706d
str x0, [sp, #8]
ldr x0, =0x6f63203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2528
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2528
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_36
_else_36:

# FAIL: comparisons
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x73
str x0, [sp, #16]
ldr x0, =0x6e6f73697261706d
str x0, [sp, #8]
ldr x0, =0x6f63203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2544
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2544
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_36:
# bool (Fake)
str x0, [sp, #-16]!
mov w2, #0
sub x4, fp, #2576
str w2, [x4]

# variable (ge)
sub x4, fp, #1120
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #1120
ldr w0, [x4]
sub x4, fp, #2576
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #2592
str w0, [x4]

# if condition
sub x4, fp, #2592
ldr w0, [x4]
cbz w0, _else_38

# PASS: 3>=5 is Fake
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x656b
str x0, [sp, #16]
ldr x0, =0x614620736920353d
str x0, [sp, #8]
ldr x0, =0x3e33203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2608
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2608
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_38
_else_38:

# FAIL: 3>=5
sub sp, sp, #16
ldr x0, =0x353d
str x0, [sp, #8]
ldr x0, =0x3e33203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #2624
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #2624
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_38:

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-80]
# start of "testLoopUntil"
.globl testLoopUntil
testLoopUntil:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #1104

sub sp, sp, #1120

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001f379c0) 

# --- loop until & ++/-- tests ---
sub sp, sp, #48

mov x0, #0
str x0, [sp, #32]

mov x0, #0
str x0, [sp, #40]
ldr x0, =0x2d2d2d2073747365
str x0, [sp, #24]
ldr x0, =0x74202d2d2f2b2b20
str x0, [sp, #16]
ldr x0, =0x26206c69746e7520
str x0, [sp, #8]
ldr x0, =0x706f6f6c202d2d2d
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
mov x2, #0
str x2, [fp, #-80]

# assign (int) load
ldr w0, [fp, #-80]

# assign (int) store
str w0, [fp, #-96]
_loop_cond_0:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
str x2, [fp, #-128]

# variable (count)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!
# comparison
ldr w0, [fp, #-96]
ldr w1, [fp, #-128]
cmp w0, w1
cset w0, lt
str w0, [fp, #-144]

# while condition
ldr w0, [fp, #-144]
cbz w0, _loop_end_0

# while-style count=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d74
str x0, [sp, #16]
ldr x0, =0x6e756f6320656c79
str x0, [sp, #8]
ldr x0, =0x74732d656c696877
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-160]

# variable (count)
ldr x0, [fp, #-96]
str x0, [sp, #-16]!

# HelloWorld arg 0
ldr x0, [fp, #-160]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-96]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# inc/dec load var
ldr w0, [fp, #-96]

# postfix: save old value
str w0, [fp, #-208]
add w0, w0, #1

# inc/dec store back
str w0, [fp, #-96]
b _loop_cond_0
_loop_end_0:

# --- for-style: 0..2 ---
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x2d2d2d20322e2e
str x0, [sp, #16]
ldr x0, =0x30203a656c797473
str x0, [sp, #8]
ldr x0, =0x2d726f66202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
str x0, [fp, #-224]

# HelloWorld arg 0
ldr x0, [fp, #-224]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
str x2, [fp, #-240]

# assign (int) load
ldr w0, [fp, #-240]

# assign (int) store
sub x4, fp, #256
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #272
str x2, [x4]

# assign default
add x0, sp, #0

# assign default store
sub x4, fp, #288
str x0, [x4]
_loop_cond_1:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #320
str x2, [x4]

# variable (i)
sub x4, fp, #256
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #256
ldr w0, [x4]
sub x4, fp, #320
ldr w1, [x4]
cmp w0, w1
cset w0, lt
sub x4, fp, #336
str w0, [x4]

# for condition
sub x4, fp, #336
ldr w0, [x4]
cbz w0, _loop_end_1

# for i=
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x3d6920726f66
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #384
str x0, [x4]

# variable (i)
sub x4, fp, #256
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #384
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #256
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# inc/dec load var
sub x4, fp, #256
ldr w0, [x4]

# postfix: save old value
sub x4, fp, #368
str w0, [x4]
add w0, w0, #1

# inc/dec store back
sub x4, fp, #256
str w0, [x4]
b _loop_cond_1
_loop_end_1:

# --- do-while: run once then until ---
sub sp, sp, #48

mov x0, #0
str x0, [sp, #40]
ldr x0, =0x2d2d2d206c
str x0, [sp, #32]
ldr x0, =0x69746e75206e6568
str x0, [sp, #24]
ldr x0, =0x742065636e6f206e
str x0, [sp, #16]
ldr x0, =0x7572203a656c6968
str x0, [sp, #8]
ldr x0, =0x772d6f64202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #416
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #416
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
sub x4, fp, #432
str x2, [x4]

# assign (int) load
sub x4, fp, #432
ldr w0, [x4]

# assign (int) store
sub x4, fp, #448
str w0, [x4]
_loop_body_2:

# do-while body n=
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3d6e2079646f6220
str x0, [sp, #8]
ldr x0, =0x656c6968772d6f64
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #512
str x0, [x4]

# variable (n)
sub x4, fp, #448
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #512
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

# inc/dec load var
sub x4, fp, #448
ldr w0, [x4]

# postfix: save old value
sub x4, fp, #560
str w0, [x4]
add w0, w0, #1

# inc/dec store back
sub x4, fp, #448
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #2
sub x4, fp, #480
str x2, [x4]

# variable (n)
sub x4, fp, #448
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #448
ldr w0, [x4]
sub x4, fp, #480
ldr w1, [x4]
cmp w0, w1
cset w0, ge
sub x4, fp, #496
str w0, [x4]

# do-while condition
sub x4, fp, #496
ldr w0, [x4]
cbnz w0, _loop_body_2
_loop_end_2:

# --- ++/-- prefix and postfix ---
sub sp, sp, #48

mov x0, #0
str x0, [sp, #32]

mov x0, #0
str x0, [sp, #40]
ldr x0, =0x2d2d2d2078696674
str x0, [sp, #24]
ldr x0, =0x736f7020646e6120
str x0, [sp, #16]
ldr x0, =0x786966657270202d
str x0, [sp, #8]
ldr x0, =0x2d2f2b2b202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #576
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #576
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #592
str x2, [x4]

# assign (int) load
sub x4, fp, #592
ldr w0, [x4]

# assign (int) store
sub x4, fp, #608
str w0, [x4]

# inc/dec load var
sub x4, fp, #608
ldr w0, [x4]

# postfix: save old value
sub x4, fp, #640
str w0, [x4]
add w0, w0, #1

# inc/dec store back
sub x4, fp, #608
str w0, [x4]

# assign (int) load
sub x4, fp, #640
ldr w0, [x4]

# assign (int) store
sub x4, fp, #656
str w0, [x4]

# a++: a=
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x3d61203a2b2b61
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #672
str x0, [x4]

# variable (a)
sub x4, fp, #608
ldr x0, [x4]
str x0, [sp, #-16]!

#  b (old)=
sub sp, sp, #16
ldr x0, =0x3d
str x0, [sp, #8]
ldr x0, =0x29646c6f28206220
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #704
str x0, [x4]

# variable (b)
sub x4, fp, #656
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #672
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #608
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #704
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 3
sub x4, fp, #656
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# inc/dec load var
sub x4, fp, #608
ldr w0, [x4]
add w0, w0, #1

# inc/dec store back
sub x4, fp, #608
str w0, [x4]

# prefix: load new value
sub x4, fp, #608
ldr w0, [x4]

# prefix: store result
sub x4, fp, #752
str w0, [x4]

# assign (int) load
sub x4, fp, #752
ldr w0, [x4]

# assign (int) store
sub x4, fp, #768
str w0, [x4]

# ++a: a=
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x3d61203a612b2b
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #784
str x0, [x4]

# variable (a)
sub x4, fp, #608
ldr x0, [x4]
str x0, [sp, #-16]!

#  c (new)=
sub sp, sp, #16
ldr x0, =0x3d
str x0, [sp, #8]
ldr x0, =0x2977656e28206320
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #816
str x0, [x4]

# variable (c)
sub x4, fp, #768
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #784
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #608
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #816
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 3
sub x4, fp, #768
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
mov x2, #10
sub x4, fp, #848
str x2, [x4]

# assign (int) load
sub x4, fp, #848
ldr w0, [x4]

# assign (int) store
sub x4, fp, #864
str w0, [x4]

# inc/dec load var
sub x4, fp, #864
ldr w0, [x4]

# postfix: save old value
sub x4, fp, #896
str w0, [x4]
sub w0, w0, #1

# inc/dec store back
sub x4, fp, #864
str w0, [x4]

# assign (int) load
sub x4, fp, #896
ldr w0, [x4]

# assign (int) store
sub x4, fp, #912
str w0, [x4]

# d--: d=
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x3d64203a2d2d64
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #928
str x0, [x4]

# variable (d)
sub x4, fp, #864
ldr x0, [x4]
str x0, [sp, #-16]!

#  e (old)=
sub sp, sp, #16
ldr x0, =0x3d
str x0, [sp, #8]
ldr x0, =0x29646c6f28206520
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #960
str x0, [x4]

# variable (e)
sub x4, fp, #912
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #928
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #864
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #960
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 3
sub x4, fp, #912
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!

# inc/dec load var
sub x4, fp, #864
ldr w0, [x4]
sub w0, w0, #1

# inc/dec store back
sub x4, fp, #864
str w0, [x4]

# prefix: load new value
sub x4, fp, #864
ldr w0, [x4]

# prefix: store result
sub x4, fp, #1008
str w0, [x4]

# assign (int) load
sub x4, fp, #1008
ldr w0, [x4]

# assign (int) store
sub x4, fp, #1024
str w0, [x4]

# --d: d=
sub sp, sp, #16

mov x0, #0
str x0, [sp, #8]
ldr x0, =0x3d64203a642d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1040
str x0, [x4]

# variable (d)
sub x4, fp, #864
ldr x0, [x4]
str x0, [sp, #-16]!

#  f (new)=
sub sp, sp, #16
ldr x0, =0x3d
str x0, [sp, #8]
ldr x0, =0x2977656e28206620
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1072
str x0, [x4]

# variable (f)
sub x4, fp, #1024
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1040
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #864
ldr x0, [x4]
bl itos
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #1072
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 3
sub x4, fp, #1024
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
str x0, [fp, #-96]
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #2144

sub sp, sp, #2160

# load parameter x from x0
str x0, [fp, #-48]

 # compound (0x600001f30240) 

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

 # compound (0x600001f30900) 
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

 # compound (0x600001f30c00) 
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

 # compound (0x600001f30d80) 
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #3
sub x4, fp, #880
str x2, [x4]

 # compound (0x600001f30e40) 
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
str x0, [sp, #16]

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

# --- Arithmetic if-checks ---
sub sp, sp, #32
ldr x0, =0x2d2d2d20
str x0, [sp, #24]
ldr x0, =0x736b636568632d66
str x0, [sp, #16]
ldr x0, =0x6920636974656d68
str x0, [sp, #8]
ldr x0, =0x74697241202d2d2d
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1248
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1248
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #52
sub x4, fp, #1280
str x2, [x4]

# variable (add)
ldr x0, [fp, #-240]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #240
ldr w0, [x4]
sub x4, fp, #1280
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1296
str w0, [x4]

# if condition
sub x4, fp, #1296
ldr w0, [x4]
cbz w0, _else_40

# PASS: 42+10==52
sub sp, sp, #16
ldr x0, =0x32353d3d30312b
str x0, [sp, #8]
ldr x0, =0x3234203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1312
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1312
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_40
_else_40:

# FAIL: 42+10
sub sp, sp, #16
ldr x0, =0x30312b
str x0, [sp, #8]
ldr x0, =0x3234203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1328
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1328
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_40:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #23
sub x4, fp, #1360
str x2, [x4]

# variable (sub)
sub x4, fp, #304
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #304
ldr w0, [x4]
sub x4, fp, #1360
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1376
str w0, [x4]

# if condition
sub x4, fp, #1376
ldr w0, [x4]
cbz w0, _else_42

# PASS: 40-17==23
sub sp, sp, #16
ldr x0, =0x33323d3d37312d
str x0, [sp, #8]
ldr x0, =0x3034203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1392
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1392
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_42
_else_42:

# FAIL: 40-17
sub sp, sp, #16
ldr x0, =0x37312d
str x0, [sp, #8]
ldr x0, =0x3034203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1408
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1408
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_42:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #100
sub x4, fp, #1440
str x2, [x4]

# variable (mul)
sub x4, fp, #368
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #368
ldr w0, [x4]
sub x4, fp, #1440
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1456
str w0, [x4]

# if condition
sub x4, fp, #1456
ldr w0, [x4]
cbz w0, _else_44

# PASS: 10*10==100
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x3030313d3d30312a
str x0, [sp, #8]
ldr x0, =0x3031203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1472
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1472
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_44
_else_44:

# FAIL: 10*10
sub sp, sp, #16
ldr x0, =0x30312a
str x0, [sp, #8]
ldr x0, =0x3031203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1488
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1488
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_44:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #5
sub x4, fp, #1520
str x2, [x4]

# variable (div)
sub x4, fp, #432
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #432
ldr w0, [x4]
sub x4, fp, #1520
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1536
str w0, [x4]

# if condition
sub x4, fp, #1536
ldr w0, [x4]
cbz w0, _else_46

# PASS: 25/5==5
sub sp, sp, #16
ldr x0, =0x353d3d352f
str x0, [sp, #8]
ldr x0, =0x3532203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1552
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1552
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_46
_else_46:

# FAIL: 25/5
sub sp, sp, #16
ldr x0, =0x352f
str x0, [sp, #8]
ldr x0, =0x3532203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1568
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1568
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_46:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #20
sub x4, fp, #1648
str x2, [x4]

# variable (mixed2)
sub x4, fp, #624
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #624
ldr w0, [x4]
sub x4, fp, #1648
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1664
str w0, [x4]
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #14
sub x4, fp, #1600
str x2, [x4]

# variable (mixed1)
sub x4, fp, #528
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #528
ldr w0, [x4]
sub x4, fp, #1600
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1616
str w0, [x4]
# logical and
sub x4, fp, #1616
ldr w0, [x4]
sub x4, fp, #1664
ldr w1, [x4]
cmp w0, #0
cset w0, ne
cmp w1, #0
cset w1, ne
and w0, w0, w1
sub x4, fp, #1680
str w0, [x4]

# if condition
sub x4, fp, #1680
ldr w0, [x4]
cbz w0, _else_48

# PASS: precedence correct
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x74636572726f6320
str x0, [sp, #16]
ldr x0, =0x65636e6564656365
str x0, [sp, #8]
ldr x0, =0x7270203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1696
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1696
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_48
_else_48:

# FAIL: precedence
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x65636e6564656365
str x0, [sp, #8]
ldr x0, =0x7270203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1712
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1712
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_48:
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #13
sub x4, fp, #1744
str x2, [x4]

# variable (nested)
sub x4, fp, #944
ldr x0, [x4]
str x0, [sp, #-16]!
# comparison
sub x4, fp, #944
ldr w0, [x4]
sub x4, fp, #1744
ldr w1, [x4]
cmp w0, w1
cset w0, eq
sub x4, fp, #1760
str w0, [x4]

# if condition
sub x4, fp, #1760
ldr w0, [x4]
cbz w0, _else_50

# PASS: ((1+2)*3)+4==13
sub sp, sp, #32

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x33313d3d34
str x0, [sp, #16]
ldr x0, =0x2b29332a29322b31
str x0, [sp, #8]
ldr x0, =0x2828203a53534150
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1776
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1776
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
b _endif_50
_else_50:

# FAIL: nested
sub sp, sp, #16
ldr x0, =0x64657473
str x0, [sp, #8]
ldr x0, =0x656e203a4c494146
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1792
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1792
ldr x0, [x4]
bl HelloWorld

# HelloWorldLine newline
bl HelloWorldLine

# store return value
str x0, [sp, #-16]!
_endif_50:

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
sub x4, fp, #1856
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
sub x4, fp, #1888
str x0, [x4]

# HelloWorld arg 0
sub x4, fp, #1856
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
ldr x0, [fp, #-144]
bl HelloWorld

# HelloWorld arg 2
sub x4, fp, #1888
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
sub x4, fp, #1920
str w0, [x4]
add sp, sp, #16

# returned value: 
sub sp, sp, #32

mov x0, #0
str x0, [sp, #16]

mov x0, #0
str x0, [sp, #24]
ldr x0, =0x203a65756c617620
str x0, [sp, #8]
ldr x0, =0x64656e7275746572
str x0, [sp, #0]

 add x0, sp, #0

# store string address
sub x4, fp, #1936
str x0, [x4]

# variable (returnVal)
sub x4, fp, #1920
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1936
ldr x0, [x4]
bl HelloWorld

# HelloWorld arg 1
sub x4, fp, #1920
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
sub x4, fp, #1968
str x0, [x4]

# load arg 0 into x0
sub x4, fp, #1968
ldr x0, [x4]

# call function stringReturn
bl stringReturn

# store return value
str x0, [sp, #-16]!

# assign call store (ptr)
sub x4, fp, #1984
str x0, [x4]
add sp, sp, #16

# variable (returnStr)
sub x4, fp, #1984
ldr x0, [x4]
str x0, [sp, #-16]!

# HelloWorld arg 0
sub x4, fp, #1984
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
sub x4, fp, #2016
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #2016
ldr x0, [x4]

# call function testStringOps
bl testStringOps

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #2032
str w0, [x4]
add sp, sp, #16
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2048
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #2048
ldr x0, [x4]

# call function testArrayOps
bl testArrayOps

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #2064
str w0, [x4]
add sp, sp, #16
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2080
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #2080
ldr x0, [x4]

# call function testBoolAndIf
bl testBoolAndIf

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #2096
str w0, [x4]
add sp, sp, #16
# integer
str x0, [sp, #-16]!
ldr x1, [sp]
mov x2, #0
sub x4, fp, #2112
str x2, [x4]

# load arg 0 into x0
sub x4, fp, #2112
ldr x0, [x4]

# call function testLoopUntil
bl testLoopUntil

# store return value
str x0, [sp, #-16]!

# assign call store (int)
sub x4, fp, #2128
str w0, [x4]
add sp, sp, #16

mov w0, #0
b return_statement

# assign default
add x0, sp, #0

# assign default store
str x0, [fp, #-112]
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
_btos_buf: .space 16
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

btos:
    stp x29, x30, [sp, #-16]!
    mov x29, sp
    adrp x3, _btos_buf@PAGE
    add x3, x3, _btos_buf@PAGEOFF
    cmp w0, #0
    b.eq btos_fake
    mov w1, #0x6552
    movk w1, #0x6c61, lsl #16
    str w1, [x3]
    strb wzr, [x3, #4]
    b btos_done
btos_fake:
    mov w1, #0x6146
    movk w1, #0x656b, lsl #16
    str w1, [x3]
    strb wzr, [x3, #4]
btos_done:
    mov x0, x3
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

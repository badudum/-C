.text
.globl _start
_start:
mov sp, fp
bl main
mov x1, x0
mov x0, #1
svc #0
 # compound (0x5e64453fba60) 
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #24sub sp, sp, #28

 # compound (0x5e64453fbd00) 
@ john do
sub sp, sp, #12
mov r0, #0
str r0, [sp, #8]
ldr r0, =06f6420
str r0, [sp, #4]
ldr r0, =06e686f6a
str r0, [sp, #0]
add r0, sp, #4
str r0, [fp, #-20]
# assign default
mov sp, [fp, -16]@ variable (name)
ldr r0, [fp, #-4]
push {r0}
@ call arg
ldr r0, [fp, #-1]
push {r0}
bl HelloWorld
add sp, sp, #0
push {r0}
# assign default
mov sp, [fp, -0]@ variable (int)
ldr r0, [fp, #-4]
push {r0}

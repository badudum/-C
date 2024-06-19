# division
ldr w0, [sp], #4
ldr w1, [sp], #4
udiv w0, w0, w1
str w0, [sp, #-4]!
ldr w1, [sp]
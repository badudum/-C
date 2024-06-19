# addition
ldr w0, [sp], #4
ldr w1, [sp]
add w0, w0, w1
str w0, [sp, #-4]!
ldr w1, [sp]
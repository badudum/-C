# access
mov w1, #%d
add x0, fp, w1
ldr w2, [%x0, #%d]
str w2, [sp, #-4]!
ldr w3, [%x0, #%d]
mov sp, w3
str sp, [fp, #%d]
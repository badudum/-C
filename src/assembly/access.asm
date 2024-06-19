# access
mov r1, #d
add r0, fp, r1
ldr r2, [r0]
push {r2}
ldr sp, [r0]
str sp, [fp, #-d]
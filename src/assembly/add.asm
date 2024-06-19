# addition
pop {r0}
ldr r1, [sp]
add r0, r0, r1
add sp, sp, #4
push {r0}
ldr r1, [sp]
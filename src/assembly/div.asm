# division
pop {r0}
pop {r1}
udiv r0, r0, r1
push {r0}
ldr r1, [sp]
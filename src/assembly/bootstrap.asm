print:
 push {fp, lr}
 mov fp, sp
 ldr r0, [fp, #8]
 bl strlen
 add sp, sp, #4
 ldr r1, [fp, #8]
 mov r2, r0
 mov r7, #4
 mov r0, #1
 mov sp, fp
 pop {fp, lr}
 svc 0
 bx lr

.type itos, %function

itos:
 push {fp, lr}
 mov fp, sp
 ldr r0, [fp, #12]           // number
 mov r1, #8
 add r2, sp, r1, lsl #1      // buffer
 mov r1, #0                  // counter
 mov r3, #0
 push {r3}
 b itos_loop 

itos_loop:
  mov r3, #0
  mov r2, #10
  udiv r3, r0, r2
  mls r0, r3, r2, r0
  add r0, r0, #48
  push {r0}
  add r1, r1, #1
  cmp r3, #0
  beq itos_buffer_loop
  b itos_loop

itos_buffer_loop: 
  pop {r0}
  strb r0, [r2, r3]
  cmp r1, #0
  beq itos_end
  add r3, r3, #1
  sub r1, r1, #1
  b itos_buffer_loop


itos_end:
  mov r0, r2
  mov sp, fp
  pop {fp, lr}
  bx lr

return_statement:
 pop {r0}
 mov sp, fp
 pop {fp, lr}
 bx lr

.type strlen, %function
strlen:
  push {fp, lr}
  mov fp, sp
  mov r1, #0
  ldr r0, [fp, #8]
  b strlenloop

strlenloop:
  ldrb r2, [r0, r1]
  cmp r2, #0
  beq strlenend
  add r1, r1, #1
  b strlenloop

strlenend:
  mov r0, r1
  mov sp, fp
  pop {fp, lr}
  bx lr
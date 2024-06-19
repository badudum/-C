HelloWorld:
  stp x29, x30, [sp, #-16]!
  mov x29, sp
  ldr x0, [sp, #16]
  bl strlen
  add sp, sp, #8
  ldr x1, [sp, #8]
  mov x2, x0
  mov x8, #4
  mov x0, #1
  mov sp, x29
  ldp x29, x30, [sp], #16
  svc #0
  ret



itos:
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  ldr w0, [sp, #16]           
  mov w1, #8
  add x2, sp, w1, uxtw #1
  mov w1, #0 
  mov w3, #0

  str wzr, [sp, #-4]!
  b itos_loop

itos_loop:
  mov w2, #0

  mov w4, #10
  udiv w0, w0, w4
  
  add w2, w2, #48
  str w2, [sp, #-4]!
  
  add w1, w1, #1

  cbz w0, itos_buffer_loop

  b itos_loop

itos_buffer_loop: 
  ldr w2, [sp], #4
  strb w2, [x2, w3, uxtw #0]

  cbz w1, itos_end

  add w3, w3, #1
  sub w1, w1, #1

  b itos_buffer_loop

itos_end:
  mov x0, x2
  mov sp, x29
  ldp x29, x30, [sp], #16
  ret

return_statement:
  ldr x0, [sp], #4
  mov sp, x29
  ldp x29, x30, [sp], #16
  ret

strlen:
  stp x29, x30, [sp, #-16]!
  mov x29, sp
  mov w1, #0
  ldr x0, [sp, #16]
  b strlenloop

strlenloop:
  ldrb w2, [x0, w1, uxtw #0]
  cmp w2, #0
  beq strlenend
  add w1, w1, #1
  b strlenloop

strlenend:
  mov x0, x1
  mov sp, x29
  ldp x29, x30, [sp], #16
  ret
  
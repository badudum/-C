.text
.globl _start
_start:
mov sp, fp
bl main
mov x1, x0
mov x0, #1
svc #0
 # compound (0x12e7066e0) 
# start of "main"
.globl main
main:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #48
sub sp, sp, #56

 # compound (0x12e706910) 

# john do
sub sp, sp, #24

mov x0, #0
str x0, [sp, #16]
ldr x0, =0x06f6420
str x0, [sp, #8]
ldr x0, =0x06e686f6a
str x0, [sp, #0]

 add x0, sp, #8
str x0, [fp, #-40]
# assign default
str x0, [x29, #-0x20]

# variable (name)
ldr x0, [fp, #-8]
str x0, [sp, #-16]!

# call arg
ldr x0, [fp, #-1]
str x0, [sp, #-16]!
bl HelloWorld
add sp, sp, #0
str x0, [sp, #-16]!
# assign default
str x0, [x29, #-0x0]

# variable (int)
ldr x0, [fp, #-8]
str x0, [sp, #-16]!
HelloWorld:
  stp x29, x30, [sp, #-16]! 
  mov x29, sp                
  sub sp, sp, #32       
  ldr x0, [x29, #16]        
  bl strlen                 
  mov x2, x0                
  ldr x1, [x29, #16]         
  mov x16, #4                
  mov x0, #1                 
  svc #0           
  mov x16, #1         
  mov sp, x29               
  ldp x29, x30, [sp], #16    
  ret      


itos:
  stp x29, x30, [sp, #-16]!
  mov x29, sp

  ldr w0, [sp, #16]           
  mov w1, #8
  add x2, sp, w1, uxtw #1
  mov w1, #0 
  mov w3, #0

  str wzr, [sp, #-8]!
  b itos_loop

itos_loop:
  mov w2, #0

  mov w4, #10
  udiv w0, w0, w4
  
  add w2, w2, #48
  str w2, [sp, #-8]!
  
  add w1, w1, #1

  cbz w0, itos_buffer_loop

  b itos_loop

itos_buffer_loop: 
  ldr w2, [sp], #8
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
  ldr x0, [sp], #8
  mov sp, x29
  ldp x29, x30, [sp], #16
  ret

strlen:
  stp x29, x30, [sp, #-16]!
  mov x29, sp
  mov w1, #0
  ldr x0, [sp, #16]
  cbz x0, strlenend
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
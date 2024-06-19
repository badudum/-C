.text
.globl _start
_start:
mov sp, fp
bl main
mov x1, x0
mov x0, #1
svc #0
 # compound (0x600001764000) 

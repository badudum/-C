.text
.globl _start
_start:
bl main
mov x0, #0
mov x16, #1
svc #0

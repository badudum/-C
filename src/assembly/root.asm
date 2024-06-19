.section .text
.global _start
_start:
    mov fp, sp
    bl main
    mov r7, r0
    mov r0, #1
    svc #0
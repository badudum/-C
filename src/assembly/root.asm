.section .text
.global _start
_start:
mov sp, fp
bl main
mov r7, #1
mov r0, r0
svc 0
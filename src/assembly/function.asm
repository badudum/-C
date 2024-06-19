# start of "%1$s"
.globl %1$s
%1$s:
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #%2$d
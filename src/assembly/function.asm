# start of "%1$s"
.global %1$s
%1$s:
push {fp, lr}
add fp, sp, #4
sub sp, sp, #d
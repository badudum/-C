#ifndef FUNCTION_ASS_X86_64_H
#define FUNCTION_ASS_X86_64_H

static const char assembly_function_begin_x86_64[] =
"# start of \"%1$s\"\n"
".globl %1$s\n"
"%1$s:\n"
"push rbp\n"
"mov rbp, rsp\n"
"sub rsp, %2$d\n";

#define assembly_function_begin_x86_64_len (sizeof(assembly_function_begin_x86_64) - 1)

#endif

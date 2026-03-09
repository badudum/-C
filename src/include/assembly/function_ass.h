#ifndef FUNCTION_ASS_H
#define FUNCTION_ASS_H

static const char assembly_function_begin_aarch64[] =
"# start of \"%1$s\"\n"
".globl %1$s\n"
"%1$s:\n"
"stp x29, x30, [sp, #-16]!\n"
"mov x29, sp\n"
"sub sp, sp, #%2$d\n";
#define assembly_function_begin_aarch64_len (sizeof(assembly_function_begin_aarch64) - 1)

static const char assembly_function_call_aarch64[] =
"# function call\n"
"# parameters %d\n"
"stp x29, x30, [sp, #-16]!\n"
"mov x29, sp\n"
"# pass parameters\n"
"# param %d\n";
#define assembly_function_call_aarch64_len (sizeof(assembly_function_call_aarch64) - 1)

static const char assembly_function_params_aarch64[] =
"# setup parameters\n"
"# param %d at offset %d\n"
"ldr x0, [fp, #%d]\n"
"str x0, [sp, #-16]!\n";
#define assembly_function_params_aarch64_len (sizeof(assembly_function_params_aarch64) - 1)

#endif

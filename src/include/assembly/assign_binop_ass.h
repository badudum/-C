static const char assembly_assignment_binop_aarch64[] =
"# assign binop\n"
"ldr w0, [fp, #%d]\n"
"str w0, [fp, #%d]\n";
#define assembly_assignment_binop_aarch64_len (sizeof(assembly_assignment_binop_aarch64) - 1)

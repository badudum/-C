static const char assembly_assignment_default_aarch64[] =
"# assign default\n"
"add x0, sp, #0\n"
"str x0, [fp, #-0x%x]\n";
#define assembly_assignment_default_aarch64_len (sizeof(assembly_assignment_default_aarch64) - 1)

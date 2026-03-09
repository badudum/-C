static const char assembly_assignment_call_aarch64[] =
"# assign call\n"
"str w0, [fp, -%d]\n"
"sub sp, sp, #%d\n";
#define assembly_assignment_call_aarch64_len (sizeof(assembly_assignment_call_aarch64) - 1)

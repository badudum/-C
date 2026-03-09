static const char assembly_int_aarch64[] =
"# integer\n"
"str x0, [sp, #-16]!\n"
"ldr x1, [sp]\n"
"mov x2, #%d\n"
"str x2, [fp, #%d]\n";
#define assembly_int_aarch64_len (sizeof(assembly_int_aarch64) - 1)

static const char assembly_int_large_offset_aarch64[] =
"# integer\n"
"str x0, [sp, #-16]!\n"
"ldr x1, [sp]\n"
"mov x2, #%d\n"
"sub x4, fp, #%d\n"
"str x2, [x4]\n";
#define assembly_int_large_offset_aarch64_len (sizeof(assembly_int_large_offset_aarch64) - 1)

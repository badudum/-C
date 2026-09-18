static const char assemble_add_aarch64[] =
"# addition\n"
"ldr w0, [fp, #%d]\n"
"ldr w1, [fp, #%d]\n"
"add w0, w0, w1\n"
"str w0, [fp, #%d]\n";
#define assemble_add_aarch64_len (sizeof(assemble_add_aarch64) - 1)

static const char assemble_add_large_offset_aarch64[] =
"# addition\n"
"sub x9, fp, #%d\n"
"ldr w0, [x9]\n"
"sub x9, fp, #%d\n"
"ldr w1, [x9]\n"
"add w0, w0, w1\n"
"sub x9, fp, #%d\n"
"str w0, [x9]\n";
#define assemble_add_large_offset_aarch64_len (sizeof(assemble_add_large_offset_aarch64) - 1)

static const char assemble_div_aarch64[] =
"# division\n"
"ldr w0, [fp, #%d]\n"
"ldr w1, [fp, #%d]\n"
"udiv w0, w0, w1\n"
"str w0, [fp, #%d]\n";
#define assemble_div_aarch64_len (sizeof(assemble_div_aarch64) - 1)

static const char assemble_div_large_offset_aarch64[] =
"# division\n"
"sub x4, fp, #%d\n"
"ldr w0, [x4]\n"
"sub x4, fp, #%d\n"
"ldr w1, [x4]\n"
"udiv w0, w0, w1\n"
"sub x4, fp, #%d\n"
"str w0, [x4]\n";
#define assemble_div_large_offset_aarch64_len (sizeof(assemble_div_large_offset_aarch64) - 1)

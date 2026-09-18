static const char assemble_div_aarch64[] =
"# division\n"
"ldr w0, [fp, #%d]\n"
"ldr w1, [fp, #%d]\n"
"sdiv w0, w0, w1\n"
"str w0, [fp, #%d]\n";
#define assemble_div_aarch64_len (sizeof(assemble_div_aarch64) - 1)

static const char assemble_div_large_offset_aarch64[] =
"# division\n"
"sub x9, fp, #%d\n"
"ldr w0, [x9]\n"
"sub x9, fp, #%d\n"
"ldr w1, [x9]\n"
"sdiv w0, w0, w1\n"
"sub x9, fp, #%d\n"
"str w0, [x9]\n";
#define assemble_div_large_offset_aarch64_len (sizeof(assemble_div_large_offset_aarch64) - 1)

static const char assemble_mul_aarch64[] =
"# multiplication\n"
"ldr w0, [fp, #%d]\n"
"ldr w1, [fp, #%d]\n"
"mul w0, w0, w1\n"
"str w0, [fp, #%d]\n";
#define assemble_mul_aarch64_len (sizeof(assemble_mul_aarch64) - 1)

static const char assemble_mul_large_offset_aarch64[] =
"# multiplication\n"
"sub x9, fp, #%d\n"
"ldr w0, [x9]\n"
"sub x9, fp, #%d\n"
"ldr w1, [x9]\n"
"mul w0, w0, w1\n"
"sub x9, fp, #%d\n"
"str w0, [x9]\n";
#define assemble_mul_large_offset_aarch64_len (sizeof(assemble_mul_large_offset_aarch64) - 1)

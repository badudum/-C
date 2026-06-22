static const char assemble_mod_aarch64[] =
"# modulus\n"
"ldr w0, [fp, #%d]\n"
"ldr w1, [fp, #%d]\n"
"sdiv w2, w0, w1\n"
"msub w0, w2, w1, w0\n"
"str w0, [fp, #%d]\n";
#define assemble_mod_aarch64_len (sizeof(assemble_mod_aarch64) - 1)

static const char assemble_mod_large_offset_aarch64[] =
"# modulus\n"
"sub x4, fp, #%d\n"
"ldr w0, [x4]\n"
"sub x4, fp, #%d\n"
"ldr w1, [x4]\n"
"sdiv w2, w0, w1\n"
"msub w0, w2, w1, w0\n"
"sub x4, fp, #%d\n"
"str w0, [x4]\n";
#define assemble_mod_large_offset_aarch64_len (sizeof(assemble_mod_large_offset_aarch64) - 1)

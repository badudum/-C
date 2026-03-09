static const char assemble_access_aarch64[] =
"# access\n"
"mov w1, #%d\n"
"add x0, fp, w1\n"
"ldr w2, [x0, #%d]\n"
"str w2, [sp, #-4]!\n"
"ldr w3, [x0, #%d]\n"
"mov sp, w3\n"
"str sp, [fp, #%d]\n";
#define assemble_access_aarch64_len (sizeof(assemble_access_aarch64) - 1)

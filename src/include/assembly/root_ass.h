static const char assemble_root_aarch64[] =
".text\n"
".globl _start\n"
"_start:\n"
"bl main\n"
"mov x0, #0\n"
"mov x16, #1\n"
"svc #0\n";
#define assemble_root_aarch64_len (sizeof(assemble_root_aarch64) - 1)

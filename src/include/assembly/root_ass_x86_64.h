static const char assemble_root_x86_64[] =
".text\n"
".code64\n"
".intel_syntax noprefix\n"
".globl _start\n"
"_start:\n"
"call main\n"
"mov rdi, 0\n"
"mov rax, 0x2000001\n"
"syscall\n";
#define assemble_root_x86_64_len (sizeof(assemble_root_x86_64) - 1)

static const char assemble_root_x86_64_linux[] =
".text\n"
".code64\n"
".intel_syntax noprefix\n"
".globl _start\n"
"_start:\n"
"call main\n"
"mov rdi, 0\n"
"mov rax, 0x2000001\n"
"syscall\n";
#define assemble_root_x86_64_linux_len (sizeof(assemble_root_x86_64_linux) - 1)

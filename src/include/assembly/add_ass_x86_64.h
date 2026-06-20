static const char assemble_add_x86_64[] =
"# addition\n"
"mov eax, [rbp-%d]\n"
"mov ebx, [rbp-%d]\n"
"add eax, ebx\n"
"mov [rbp-%d], eax\n";
#define assemble_add_x86_64_len (sizeof(assemble_add_x86_64) - 1)

static const char assemble_add_large_offset_x86_64[] =
"# addition\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov eax, [rcx]\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov ebx, [rcx]\n"
"add eax, ebx\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov [rcx], eax\n";
#define assemble_add_large_offset_x86_64_len (sizeof(assemble_add_large_offset_x86_64) - 1)

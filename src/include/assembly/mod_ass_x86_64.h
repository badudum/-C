static const char assemble_mod_x86_64[] =
"# modulus\n"
"mov eax, [rbp-%d]\n"
"mov ebx, [rbp-%d]\n"
"cdq\n"
"idiv ebx\n"
"mov [rbp-%d], edx\n";
#define assemble_mod_x86_64_len (sizeof(assemble_mod_x86_64) - 1)

static const char assemble_mod_large_offset_x86_64[] =
"# modulus\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov eax, [rcx]\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov ebx, [rcx]\n"
"cdq\n"
"idiv ebx\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov [rcx], edx\n";
#define assemble_mod_large_offset_x86_64_len (sizeof(assemble_mod_large_offset_x86_64) - 1)

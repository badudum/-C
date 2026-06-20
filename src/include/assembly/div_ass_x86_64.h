static const char assemble_div_x86_64[] =
"# division\n"
"mov eax, [rbp-%d]\n"
"mov ebx, [rbp-%d]\n"
"cdq\n"
"idiv ebx\n"
"mov [rbp-%d], eax\n";
#define assemble_div_x86_64_len (sizeof(assemble_div_x86_64) - 1)

static const char assemble_div_large_offset_x86_64[] =
"# division\n"
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
"mov [rcx], eax\n";
#define assemble_div_large_offset_x86_64_len (sizeof(assemble_div_large_offset_x86_64) - 1)

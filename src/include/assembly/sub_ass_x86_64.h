static const char assemble_sub_x86_64[] =
"# subtraction\n"
"mov eax, [rbp-%d]\n"
"mov ebx, [rbp-%d]\n"
"sub eax, ebx\n"
"mov [rbp-%d], eax\n";
#define assemble_sub_x86_64_len (sizeof(assemble_sub_x86_64) - 1)

static const char assemble_sub_large_offset_x86_64[] =
"# subtraction\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov eax, [rcx]\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov ebx, [rcx]\n"
"sub eax, ebx\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov [rcx], eax\n";
#define assemble_sub_large_offset_x86_64_len (sizeof(assemble_sub_large_offset_x86_64) - 1)

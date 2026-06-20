static const char assemble_mul_x86_64[] =
"# multiplication\n"
"mov eax, [rbp-%d]\n"
"mov ebx, [rbp-%d]\n"
"imul eax, ebx\n"
"mov [rbp-%d], eax\n";
#define assemble_mul_x86_64_len (sizeof(assemble_mul_x86_64) - 1)

static const char assemble_mul_large_offset_x86_64[] =
"# multiplication\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov eax, [rcx]\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov ebx, [rcx]\n"
"imul eax, ebx\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov [rcx], eax\n";
#define assemble_mul_large_offset_x86_64_len (sizeof(assemble_mul_large_offset_x86_64) - 1)

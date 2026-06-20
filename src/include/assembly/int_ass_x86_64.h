static const char assembly_int_x86_64[] =
"# integer\n"
"sub rsp, 16\n"
"mov eax, %d\n"
"mov [rbp-%d], eax\n";
#define assembly_int_x86_64_len (sizeof(assembly_int_x86_64) - 1)

static const char assembly_int_large_offset_x86_64[] =
"# integer\n"
"sub rsp, 16\n"
"mov eax, %d\n"
"mov rcx, rbp\n"
"sub rcx, %d\n"
"mov [rcx], eax\n";
#define assembly_int_large_offset_x86_64_len (sizeof(assembly_int_large_offset_x86_64) - 1)

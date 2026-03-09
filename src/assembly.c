
#include "include/assembly.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "include/assembly/function_ass.h"
#include "include/assembly/call_ass.h"
#include "include/assembly/assign_int_ass.h"
#include "include/assembly/assign_call_ass.h"
#include "include/assembly/assign_binop_ass.h"
#include "include/assembly/assign_default_ass.h"
#include "include/assembly/int_ass.h"
#include "include/assembly/root_ass.h"
#include "include/assembly/bootstrap_ass.h"
#include "include/assembly/access_ass.h"
#include "include/assembly/add_ass.h"
#include "include/assembly/sub_ass.h"
#include "include/assembly/mul_ass.h"
#include "include/assembly/div_ass.h"


/*
* The reason we allocate an additional 128 bytes is to ensure that we have enough space, and does not cause a buffer overflow
*/

/* Helper: emit ldr w0 from [fp, offset]. Handles large offsets. */
static void append_load_w_from_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nldr w0, [fp, #%d]\n", comment, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nldr w0, [x4]\n", comment, abs_off);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

/* Helper: emit str w0 to [fp, offset]. Handles large offsets. */
static void append_store_w_to_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nstr w0, [fp, #%d]\n", comment, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nstr w0, [x4]\n", comment, abs_off);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

char * assemble_compound(AST_t* ast, dynamic_list_t* list)
{
    const char* template = "\n # compound (%p) \n";
    char * value = calloc(strlen(template) + 128, sizeof(char));
    sprintf(value, template, ast);
    for (unsigned int i = 0; i < ast->children->size; i ++)
    {
        AST_t* child = (AST_t*) ast->children->items[i];
        char * next = assemble(child, list);
        value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
        strcat(value, next);
    }
    return value;
}


// this is the assembly conversion for assignment occurrences
char * assemble_assignment(AST_t * ast, dynamic_list_t * list)
{
    int id = (ast->stack_index * 16);

    char* s = calloc(1, sizeof(char));

    list_enqueue(list, ast);

    char* assemble_value = assemble(ast->parent , list);


    if(assemble_value)
    {
        s = realloc(s, (strlen(s) + strlen(assemble_value) + 1) * sizeof(char));
        strcat(s, assemble_value);
        free(assemble_value);
    }

    if(ast->datatype == TYPE_INT)
    {
        AST_t* rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t*) rhs->children->items[0];
        int rhs_offset = rhs->stack_index * -16;
        int var_offset = ast->stack_index * -16;
        append_load_w_from_fp(&s, rhs_offset, "assign (int) load");
        append_store_w_to_fp(&s, var_offset, "assign (int) store");
    }

    else if (ast->parent->type == CALL_AST)
    {
        char * mo = calloc(assembly_assignment_call_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_call_aarch64, id, 0);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }

    else if(ast->parent->type == BINOP_AST || (ast->parent->type == COMP_AST && ast->parent->children->size == 1))
    {
        AST_t* rhs = ast->parent->type == COMP_AST ? (AST_t*) ast->parent->children->items[0] : ast->parent;
        int rhs_offset = rhs->stack_index * -16;
        int var_offset = ast->stack_index * -16;
        append_load_w_from_fp(&s, rhs_offset, "assign binop load");
        append_store_w_to_fp(&s, var_offset, "assign binop store");
    }

    else 
    {
        char * mo = calloc(assembly_assignment_default_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_default_aarch64, id);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }
    return s;
}

/* Helper: emit store from reg to [fp, offset]. Handles large offsets. */
static void append_store_to_fp(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nstr x%d, [fp, #%d]\n", comment, reg, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nstr x%d, [x4]\n", comment, abs_off, reg);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

/* Helper: emit load from [fp, offset] into reg. Handles large offsets. */
static void append_load_from_fp(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nldr x%d, [fp, #%d]\n", comment, reg, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nldr x%d, [x4]\n", comment, abs_off, reg);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

// making a variable
char * assemble_variable(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));
    char cmt[64];
    snprintf(cmt, sizeof(cmt), "variable (%s)", ast->name ? ast->name : "");
    append_load_from_fp(&s, ast->stack_index * -16, 0, cmt);
    const char* push = "str x0, [sp, #-16]!\n";
    s = realloc(s, (strlen(s) + strlen(push) + 1) * sizeof(char));
    strcat(s, push);
    return s;
}

// making a function call
char * assemble_call(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    // Assemble each argument first (so literals like 42 get stored in stack slots)
    unsigned int num_args = ast->parent->children->size;
    for (unsigned int i = 0; i < num_args; i++) {
        AST_t* arg = (AST_t*) ast->parent->children->items[i];
        char* arg_asm = assemble(arg, list);
        s = realloc(s, (strlen(s) + strlen(arg_asm) + 1) * sizeof(char));
        strcat(s, arg_asm);
        free(arg_asm);
    }

    // Calculate stack space needed for arguments beyond the first 8
    unsigned int stack_args = (num_args > 8) ? (num_args - 8) : 0;
    unsigned int total_arg_size = stack_args * 16;

    // Reserve space on stack for additional arguments (beyond the first 8)
    if(stack_args > 0)
    {
        const char* reserve_template = "\n# reserve space for %d stack arguments\n"
                                       "sub sp, sp, #%d\n";
        char * reserve = calloc(strlen(reserve_template) + 128, sizeof(char));
        sprintf(reserve, reserve_template, stack_args, total_arg_size);
        s = realloc(s, (strlen(s) + strlen(reserve) + 1) * sizeof(char));
        strcat(s, reserve);
        free(reserve);
    }

    int is_hello_world = (strcmp(ast->name, "HelloWorld") == 0);
    int is_hello_world_line = (strcmp(ast->name, "HelloWorldLine") == 0);

    if (is_hello_world || is_hello_world_line) {
        // HelloWorld: print each argument (load into x0, call for each)
        for (unsigned int i = 0; i < num_args; i++) {
            AST_t* arg = (AST_t*) ast->parent->children->items[i];
            int need_itos = 0;
            if (arg->type == INT_AST || arg->type == BINOP_AST)
                need_itos = 1;
            else if (arg->type == VAR_AST) {
                for (unsigned int j = 0; j < list->size; j++) {
                    AST_t* def = (AST_t*) list->items[j];
                    if (def->type == ASSIGNEMENT_AST && def->name && arg->name &&
                        strcmp(def->name, arg->name) == 0 && def->datatype == TYPE_INT) {
                        need_itos = 1;
                        break;
                    }
                }
            }
            int off = arg->stack_index * -16;
            char cmt[64];
            snprintf(cmt, sizeof(cmt), "HelloWorld arg %d", i);
            append_load_from_fp(&s, off, 0, cmt);
            if (need_itos) {
                const char* itos = "bl itos\n";
                s = realloc(s, (strlen(s) + strlen(itos) + 1) * sizeof(char));
                strcat(s, itos);
            }
            const char* call_one = "bl HelloWorld\n";
            s = realloc(s, (strlen(s) + strlen(call_one) + 1) * sizeof(char));
            strcat(s, call_one);
        }
        if (is_hello_world_line) {
            const char* newline_call = "\n# HelloWorldLine newline\nbl HelloWorldLine\n";
            s = realloc(s, (strlen(s) + strlen(newline_call) + 1) * sizeof(char));
            strcat(s, newline_call);
        }
    } else {
        // Pass arguments: first 8 in registers x0-x7, rest on stack
        for(int i = 0; i < num_args; i++)
        {
            AST_t* arg = (AST_t*) ast->parent->children->items[i];

            if(i < 8)
            {
                int off = arg->stack_index * -16;
                char cmt[64];
                snprintf(cmt, sizeof(cmt), "load arg %d into x%d", i, i);
                append_load_from_fp(&s, off, i, cmt);
            }
            else
            {
                int stack_offset = (i - 8) * 16;
                int off = arg->stack_index * -16;
                char cmt[64];
                snprintf(cmt, sizeof(cmt), "load arg %d for stack", i);
                append_load_from_fp(&s, off, 0, cmt);
                const char* store_template = "\n# store arg on stack\nstr x0, [sp, #%d]\n";
                char * store_str = calloc(strlen(store_template) + 64, sizeof(char));
                sprintf(store_str, store_template, stack_offset);
                s = realloc(s, (strlen(s) + strlen(store_str) + 1) * sizeof(char));
                strcat(s, store_str);
                free(store_str);
            }
        }

        // Make the function call
        const char* call_template = "\n# call function %s\nbl %s\n";
        char * call = calloc(strlen(call_template) + 128, sizeof(char));
        sprintf(call, call_template, ast->name, ast->name);
        s = realloc(s, (strlen(s) + strlen(call) + 1) * sizeof(char));
        strcat(s, call);
        free(call);
    }

    // Clean up stack space used for additional arguments
    if(stack_args > 0)
    {
        const char* cleanup_template = "\n# cleanup stack argument space\n"
                                       "add sp, sp, #%d\n";
        char * cleanup = calloc(strlen(cleanup_template) + 128, sizeof(char));
        sprintf(cleanup, cleanup_template, total_arg_size);
        s = realloc(s, (strlen(s) + strlen(cleanup) + 1) * sizeof(char));
        strcat(s, cleanup);
        free(cleanup);
    }

    // Store the return value
    const char* store_return_template = "\n# store return value\n"
                                        "str x0, [sp, #-16]!\n";
    s = realloc(s, (strlen(s) + strlen(store_return_template) + 1) * sizeof(char));
    strcat(s, store_return_template);

    return s;
}


char * assemble_int(AST_t * ast, dynamic_list_t * list)
{
    int index = ast->stack_index * -16;
    int abs_offset = index < 0 ? -index : index;

    char * s;
    if (abs_offset <= 255) {
        s = calloc(assembly_int_aarch64_len + 128, sizeof(char));
        sprintf(s, (char*) assembly_int_aarch64, ast->int_value, index);
    } else {
        s = calloc(assembly_int_large_offset_aarch64_len + 128, sizeof(char));
        sprintf(s, (char*) assembly_int_large_offset_aarch64, ast->int_value, abs_offset);
    }
    return s;
}


char * assemble_string(AST_t * ast, dynamic_list_t * list)
{
    dynamic_list_t * chunks = str_to_hex_list(ast->string_value);
    unsigned int string_len = strlen(ast->string_value);
    unsigned int min_bytes = string_len + 1;  // string + null terminator
    // Simply round the minimum required bytes to 16-byte boundary
    unsigned int numb_bytes = ((min_bytes + 15) / 16) * 16;
    unsigned int diff = numb_bytes - min_bytes;


    printf("String '%s': length=%d, chunks=%d, allocated=%d bytes\n", ast->string_value, string_len, chunks->size, numb_bytes);


    // Start placing string data from the end, working backwards to offset 0
    unsigned int byte_counter = (chunks->size - 1) * 8;

    // int index = ast->stack_index * 8;
    int index = ast->stack_index * -16;

    /* Sanitize string for comment: newlines/tabs would break the assembler */
    char * comment_safe = calloc(string_len + 1, sizeof(char));
    for (unsigned int i = 0; i < string_len; i++) {
        char c = ast->string_value[i];
        comment_safe[i] = (c == '\n' || c == '\r' || c == '\t') ? ' ' : c;
    }
    comment_safe[string_len] = '\0';

    const char* subl_template = "\n# %s\n"
                                "sub sp, sp, #%d\n";

    char * sub = calloc(strlen(subl_template) + string_len + 128, sizeof(char));
    sprintf(sub, subl_template, comment_safe, numb_bytes);
    free(comment_safe);

    char * strpush = calloc(strlen(sub)+1, sizeof(char));
    strcat(strpush, sub);

    // Place null terminator at the end if needed
    if (numb_bytes > (chunks->size * 8)) {
        const char*  push_zero = "\nmov x0, #0\n"
                                 "str x0, [sp, #%d]\n";
        
        char * zero = calloc(strlen(push_zero) + 128, sizeof(char));
        sprintf(zero, push_zero, numb_bytes - 8);
        strpush = realloc(strpush, (strlen(strpush) + strlen(zero) + 1) * sizeof(char));
        strcat(strpush, zero);
    }  

    const char* pushtemplate = "ldr x0, =0x%s\n"
                           "str x0, [sp, #%d]\n";

    for(unsigned int i = 0; i < chunks->size ; i ++)
    {
        char * push_hex = (char*) chunks->items[(chunks->size - i)-1];
        printf("%s\n", push_hex);
        char * push = calloc(strlen(pushtemplate) + strlen(push_hex) + 1, sizeof(char));
        sprintf(push, pushtemplate, push_hex, byte_counter);
        strpush = realloc(strpush, (strlen(strpush) + strlen(push) + 1) * sizeof(char));
        strcat(strpush, push);
        free(push);
        free(push_hex);
        if (byte_counter >= 8) {
            byte_counter -= 8;
        }
    }

    const char * add_template = "\n add x0, sp, #%d\n";
    char * add_instr = calloc(strlen(add_template) + 64, sizeof(char));
    sprintf(add_instr, add_template, 0);
    strpush = realloc(strpush, (strlen(strpush) + strlen(add_instr) + 1) * sizeof(char));
    strcat(strpush, add_instr);
    free(add_instr);

    append_store_to_fp(&strpush, index, 0, "store string address");

    return strpush;
}

/*
* This function is used to assemble the root of the AST
    Include Section Text:
        It starts by copying the section text from src_asm_root_asm into a buffer.
        This section text is defined in the root.h file and contains the initial assembly instructions.
    Process the AST:
        It then processes the AST by calling the as_f function, which generates the assembly code for the given AST node and its children.
        The generated code is appended to the buffer.
    Append Bootstrap Assembly Code:
        Finally, it appends the bootstrap assembly code from src_asm_bootstrap_asm to the buffer.
         This bootstrap code is necessary for setting up the initial state of the program.

*/
char * assemble_root(AST_t* ast, dynamic_list_t * list)
{
    const char * section = (char*) assemble_root_aarch64;
    char* value = calloc(assemble_root_aarch64_len + 128, sizeof(char));
    sprintf(value, "%s", section);

    char* next = assemble(ast, list);

    value = (char *)realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
    strcat(value, next);

    value = realloc(value, (strlen(value) + assembly_bootstrap_aarch64_len + 1) * sizeof(char));
    strcat(value, (char*) assembly_bootstrap_aarch64);

    return value;
}


char * assemble_binop(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    char * left_str = assemble(ast->left, list);
    char * right_str = assemble(ast->right, list);

    s = realloc(s, (strlen(left_str) + strlen(right_str) + 1) * sizeof(char));
    strcat(s, right_str);
    strcat(s, left_str);

    int left_offset = ast->left->stack_index * -16;
    int right_offset = ast->right->stack_index * -16;
    int result_offset = ast->stack_index * -16;
    int left_abs = left_offset < 0 ? -left_offset : left_offset;
    int right_abs = right_offset < 0 ? -right_offset : right_offset;
    int result_abs = result_offset < 0 ? -result_offset : result_offset;
    int use_large = (left_abs > 255 || right_abs > 255 || result_abs > 255);

    char * op_asm = calloc(512, sizeof(char));
    switch(ast->op)
    {
        case PLUS_TOKEN:
            if (use_large)
                sprintf(op_asm, (char*) assemble_add_large_offset_aarch64, left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, (char*) assemble_add_aarch64, left_offset, right_offset, result_offset);
            break;
        case MINUS_TOKEN:
            if (use_large)
                sprintf(op_asm, (char*) assemble_sub_large_offset_aarch64, left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, (char*) assemble_sub_aarch64, left_offset, right_offset, result_offset);
            break;
        case ASTERISK_TOKEN:
            if (use_large)
                sprintf(op_asm, (char*) assemble_mul_large_offset_aarch64, left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, (char*) assemble_mul_aarch64, left_offset, right_offset, result_offset);
            break;
        case SLASH_TOKEN:
            if (use_large)
                sprintf(op_asm, (char*) assemble_div_large_offset_aarch64, left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, (char*) assemble_div_aarch64, left_offset, right_offset, result_offset);
            break;
        default:
            printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type);
            exit(1);
    }

    s = realloc(s, (strlen(s) + strlen(op_asm) + 1) * sizeof(char));
    strcat(s, op_asm);
    free(op_asm);
    return s;
}


char * assemble_access(AST_t * ast, dynamic_list_t * list)
{
    int offset = ((ast->stack_index * -1) * 8) - 8;
    int array_offset = MAX(8, (ast->int_value+1) *8);

    char * s = calloc(assemble_access_aarch64_len + 128, sizeof(char));
    sprintf(s, (char*) assemble_access_aarch64, offset, array_offset, array_offset, ast->stack_index * 8);

    return s;
}


char * assemble_return(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    const char* template = "\n%s\n"
                            "b return_statement\n";

    char * value = assemble(ast->parent, list);
    char * ret = calloc(strlen(template) + strlen(value) + 128, sizeof(char));
    sprintf(ret, template, value);

    s = realloc(s, (strlen(ret) + 1) * sizeof(char));
    strcat(s, ret);
    free(ret);

    return s;
}


char * assemble_function(AST_t* ast, dynamic_list_t* list)
{
    char * name = ast->name;
    int index = ast->stackframe->stack->size * 16;

    char * s = calloc((assembly_function_begin_aarch64_len + (strlen(name)) + 1), sizeof(char));

    sprintf(s, (char*) assembly_function_begin_aarch64, name, index);

    if(ast->stackframe->stack->size)
    {
        const char* sub_template = "\nsub sp, sp, #%d\n";
        char* sub = calloc(strlen(sub_template) + 128, sizeof(char));
        sprintf(sub, sub_template, (1 + ast->stackframe->stack->size) * 16);

        s = realloc(s, (strlen(s) + strlen(sub) + 1) * sizeof(char));
        strcat(s, sub);
        free(sub);
    }

    AST_t* assembly_value = ast;

    // Set up function parameters - first 8 parameters in registers x0-x7, rest on stack
    for (unsigned int i = 0; i < assembly_value->children->size; i++)
    {
        AST_t* function_arg = (AST_t*) assembly_value->children->items[i];

        if(i < 8)
        {
            // First 8 parameters are passed in registers x0-x7
            const char* param_template = "\n# load parameter %s from x%d\n"
                                         "str x%d, [fp, #%d]\n";
            char* param_setup = calloc(strlen(param_template) + 128, sizeof(char));
            sprintf(param_setup, param_template, function_arg->name, i, i, function_arg->stack_index * -16);

            s = realloc(s, (strlen(s) + strlen(param_setup) + 1) * sizeof(char));
            strcat(s, param_setup);
            free(param_setup);
        }
        else
        {
            // Additional parameters are passed on the stack
            int stack_offset = 16 + (i - 8) * 16; // Start after return address/frame pointer
            const char* param_template = "\n# load parameter %s from stack\n"
                                         "ldr x0, [sp, #%d]\n"
                                         "str x0, [fp, #%d]\n";
            char* param_setup = calloc(strlen(param_template) + 128, sizeof(char));
            sprintf(param_setup, param_template, function_arg->name, stack_offset, function_arg->stack_index * -16);

            s = realloc(s, (strlen(s) + strlen(param_setup) + 1) * sizeof(char));
            strcat(s, param_setup);
            free(param_setup);
        }
    }

    char* assemble_value_value = assemble(assembly_value->parent, list);
    s = realloc(s, (strlen(s) + strlen(assemble_value_value) + 1) * sizeof(char));
    strcat(s, assemble_value_value);
    free(assemble_value_value);

    return s;
}






char * assemble(AST_t * ast, dynamic_list_t * list)
{
    // printf("Current : %d", ast->type);
    char * value = calloc(1, sizeof(char));

    char * next = 0;

    switch(ast->type)
    {
        case COMP_AST : next = assemble_compound(ast, list);break;
        case ASSIGNEMENT_AST : next = assemble_assignment(ast, list);break;
        case VAR_AST : next = assemble_variable(ast, list);break;
        case CALL_AST : next = assemble_call(ast, list);break;
        case INT_AST : next = assemble_int(ast, list); break;
        case STRING_AST : next = assemble_string(ast, list); break;
        case BINOP_AST : next = assemble_binop(ast, list); break;
        case ACCESS_AST : next = assemble_access(ast, list); break;
        case RETURN_AST : next = assemble_return(ast, list); break;
        case FUNC_AST : next = assemble_function(ast, list); break;
        default : {printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type); exit(1);} break;

    }

    value = realloc(value, (strlen(next) +1 ) * sizeof(char));
    strcat(value, next);

    return value;
}
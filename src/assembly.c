
#include "include/assembly.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "include/assembly/function_ass.h"
#include "include/assembly/call_ass.h"
#include "include/assembly/assign_int_ass.h"
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

/* Forward declaration */
static void append_load_from_fp(char **s, int offset, int reg, const char *comment);
char * assemble(AST_t * ast, dynamic_list_t * list);

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

/* Byte load/store for booleans (1 byte per value at same slot offset). */
static void append_load_b_from_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nldrb w0, [fp, #%d]\n", comment, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nldrb w0, [x4]\n", comment, abs_off);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

static void append_store_b_to_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nstrb w0, [fp, #%d]\n", comment, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nstrb w0, [x4]\n", comment, abs_off);
    }
    *s = realloc(*s, (strlen(*s) + strlen(instr) + 1) * sizeof(char));
    strcat(*s, instr);
    free(instr);
}

/* Helper: emit str xN to [fp, offset]. Handles large offsets. */
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

/* Helper: return store instruction for xN to [fp, offset]. Caller frees. */
static char* store_x_to_fp_instr(int offset, int reg, const char* comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char *instr = calloc(256, sizeof(char));
    if (abs_off <= 255) {
        sprintf(instr, "\n# %s\nstr x%d, [fp, #%d]\n", comment, reg, offset);
    } else {
        sprintf(instr, "\n# %s\nsub x4, fp, #%d\nstr x%d, [x4]\n", comment, abs_off, reg);
    }
    return instr;
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

    /* += and -=: load rhs into w0, move to w1, load var into w0, add/sub (w0 = w0 op w1), store */
    if (ast->op == PLUS_EQUALS_TOKEN || ast->op == MINUS_EQUALS_TOKEN) {
        AST_t* rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t*) rhs->children->items[0];
        int var_offset = ast->stack_index * -16;
        int rhs_offset = rhs->stack_index * -16;
        append_load_w_from_fp(&s, rhs_offset, "+=/-= load rhs");
        { const char* mov = "mov w1, w0\n"; s = realloc(s, strlen(s) + strlen(mov) + 1); strcat(s, mov); }
        append_load_w_from_fp(&s, var_offset, "+=/-= load var");
        if (ast->op == PLUS_EQUALS_TOKEN) {
            const char* add_op = "add w0, w0, w1\n";
            s = realloc(s, strlen(s) + strlen(add_op) + 1);
            strcat(s, add_op);
        } else {
            const char* sub_op = "sub w0, w0, w1\n";
            s = realloc(s, strlen(s) + strlen(sub_op) + 1);
            strcat(s, sub_op);
        }
        append_store_w_to_fp(&s, var_offset, "+=/-= store");
        return s;
    }

    if (ast->parent->type == CALL_AST)
    {
        /* Return value is in x0 after the call; store to variable and pop stack */
        int var_offset = ast->stack_index * -16;
        if (ast->datatype == TYPE_INT)
            append_store_w_to_fp(&s, var_offset, "assign call store (int)");
        else if (ast->datatype == TYPE_BOOL)
            append_store_b_to_fp(&s, var_offset, "assign call store (bool)");
        else
            append_store_to_fp(&s, var_offset, 0, "assign call store (ptr)");
        const char* pop = "add sp, sp, #16\n";
        s = realloc(s, (strlen(s) + strlen(pop) + 1) * sizeof(char));
        strcat(s, pop);
    }

    else if (IS_ARRAY_TYPE(ast->datatype))
    {
        /* Array literal: elements already stored by assemble_array_literal.
           Store the base stack_index in the variable slot so access can find it. */
        AST_t* rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t*) rhs->children->items[0];
        int var_offset = ast->stack_index * -16;
        char *arr_info = calloc(256, sizeof(char));
        int base_slot = rhs->stack_index;
        int count = rhs->int_value;
        sprintf(arr_info, "\n# assign array (base_slot=%d, count=%d)\nmov w0, #%d\n",
                base_slot, count, base_slot);
        s = realloc(s, strlen(s) + strlen(arr_info) + 1);
        strcat(s, arr_info);
        free(arr_info);
        append_store_w_to_fp(&s, var_offset, "store array base slot");
    }

    else if(ast->datatype == TYPE_INT || ast->datatype == TYPE_BOOL)
    {
        AST_t* rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t*) rhs->children->items[0];
        int var_offset = ast->stack_index * -16;
        if (rhs->type == ACCESS_AST) {
            /* Array element access pushed result to stack */
            const char* pop = "\n# assign int (pop from stack)\nldr x0, [sp], #16\n";
            s = realloc(s, strlen(s) + strlen(pop) + 1);
            strcat(s, pop);
            append_store_w_to_fp(&s, var_offset, "assign int from access");
        } else {
            int rhs_offset = rhs->stack_index * -16;
            if (ast->datatype == TYPE_BOOL) {
                append_load_b_from_fp(&s, rhs_offset, "assign (bool) load");
                append_store_b_to_fp(&s, var_offset, "assign (bool) store");
            } else {
                append_load_w_from_fp(&s, rhs_offset, "assign (int) load");
                append_store_w_to_fp(&s, var_offset, "assign (int) store");
            }
        }
    }

    else if(ast->parent->type == BINOP_AST || (ast->parent->type == COMP_AST && ast->parent->children->size == 1))
    {
        AST_t* rhs = ast->parent->type == COMP_AST ? (AST_t*) ast->parent->children->items[0] : ast->parent;
        int rhs_offset = rhs->stack_index * -16;
        int var_offset = ast->stack_index * -16;
        if (ast->datatype == TYPE_STR) {
            /* String BINOP (+ ) pushes result to stack; pop and store */
            const char* pop_str = "\n# assign str binop (pop from stack)\nldr x0, [sp], #16\n";
            s = realloc(s, strlen(s) + strlen(pop_str) + 1);
            strcat(s, pop_str);
            append_store_to_fp(&s, var_offset, 0, "assign binop store (str)");
        } else {
            if (ast->datatype == TYPE_BOOL) {
                append_load_b_from_fp(&s, rhs_offset, "assign binop load (bool)");
                append_store_b_to_fp(&s, var_offset, "assign binop store (bool)");
            } else {
                append_load_w_from_fp(&s, rhs_offset, "assign binop load");
                append_store_w_to_fp(&s, var_offset, "assign binop store");
            }
        }
    }

    else if (ast->datatype == TYPE_STR)
    {
        AST_t* rhs = ast->parent->type == COMP_AST && ast->parent->children->size == 1
            ? (AST_t*) ast->parent->children->items[0] : ast->parent;
        int var_offset = ast->stack_index * -16;
        /* SLICE, ACCESS push result; STRING_AST stores to fp */
        if (rhs->type == SLICE_AST || rhs->type == ACCESS_AST) {
            const char* pop_str = "\n# assign str (pop from stack)\nldr x0, [sp], #16\n";
            s = realloc(s, strlen(s) + strlen(pop_str) + 1);
            strcat(s, pop_str);
        } else {
            int rhs_offset = rhs->stack_index * -16;
            append_load_from_fp(&s, rhs_offset, 0, "assign str load");
        }
        append_store_to_fp(&s, var_offset, 0, "assign str store");
    }

    else 
    {
        int var_offset = ast->stack_index * -16;
        const char* add_sp = "\n# assign default\nadd x0, sp, #0\n";
        s = realloc(s, strlen(s) + strlen(add_sp) + 1);
        strcat(s, add_sp);
        append_store_to_fp(&s, var_offset, 0, "assign default store");
    }
    return s;
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
        for (unsigned int i = 0; i < num_args; i++) {
            AST_t* arg = (AST_t*) ast->parent->children->items[i];
            int need_itos = 0;
            int need_btos = 0;
            if (arg->type == INT_AST)
                need_itos = 1;
            else if (arg->type == BOOL_AST)
                need_btos = 1;
            else if (arg->type == BINOP_AST) {
                if (arg->op == DEQUALS_TOKEN || arg->op == NOT_EQUALS_TOKEN ||
                    arg->op == LT_TOKEN || arg->op == GT_TOKEN ||
                    arg->op == LTE_TOKEN || arg->op == GTE_TOKEN ||
                    arg->op == AND_TOKEN || arg->op == OR_TOKEN)
                    need_btos = 1;
                else
                    need_itos = 1;
            }
            else if (arg->type == UNARY_AST) {
                if (arg->op == NOT_TOKEN)
                    need_btos = 1;
                else
                    need_itos = 1;
            }
            else if (arg->type == VAR_AST) {
                for (unsigned int j = 0; j < list->size; j++) {
                    AST_t* def = (AST_t*) list->items[j];
                    if (def->type == ASSIGNEMENT_AST && def->name && arg->name &&
                        strcmp(def->name, arg->name) == 0) {
                        if (def->datatype == TYPE_INT)
                            need_itos = 1;
                        else if (def->datatype == TYPE_BOOL)
                            need_btos = 1;
                        break;
                    }
                }
            }
            int off = arg->stack_index * -16;
            char cmt[64];
            snprintf(cmt, sizeof(cmt), "HelloWorld arg %d", i);
            if (need_btos) {
                append_load_b_from_fp(&s, off, cmt);
            } else {
                append_load_from_fp(&s, off, 0, cmt);
            }
            if (need_btos) {
                static int btos_id = 0;
                int bid = btos_id++;
                char btos_buf[512];
                sprintf(btos_buf,
                    "cmp w0, #0\n"
                    "b.eq _btos_fake_%d\n"
                    "sub sp, sp, #16\n"
                    "mov w1, #0x6552\n"
                    "movk w1, #0x6c61, lsl #16\n"
                    "str w1, [sp]\n"
                    "strb wzr, [sp, #4]\n"
                    "mov x1, sp\n"
                    "mov x2, #4\n"
                    "mov x0, #1\n"
                    "mov x16, #4\n"
                    "svc #0\n"
                    "add sp, sp, #16\n"
                    "b _btos_end_%d\n"
                    "_btos_fake_%d:\n"
                    "sub sp, sp, #16\n"
                    "mov w1, #0x6146\n"
                    "movk w1, #0x656b, lsl #16\n"
                    "str w1, [sp]\n"
                    "strb wzr, [sp, #4]\n"
                    "mov x1, sp\n"
                    "mov x2, #4\n"
                    "mov x0, #1\n"
                    "mov x16, #4\n"
                    "svc #0\n"
                    "add sp, sp, #16\n"
                    "_btos_end_%d:\n",
                    bid, bid, bid, bid);
                s = realloc(s, strlen(s) + strlen(btos_buf) + 1);
                strcat(s, btos_buf);
                goto skip_hello_call;
            } else if (need_itos) {
                const char* itos = "bl itos\n";
                s = realloc(s, (strlen(s) + strlen(itos) + 1) * sizeof(char));
                strcat(s, itos);
            }
            const char* call_one = "bl HelloWorld\n";
            s = realloc(s, (strlen(s) + strlen(call_one) + 1) * sizeof(char));
            strcat(s, call_one);
            skip_hello_call:;
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

    // Zero all bytes from after chunk data to end of allocated region
    for (unsigned int z = chunks->size * 8; z < numb_bytes; z += 8) {
        const char*  push_zero = "\nmov x0, #0\n"
                                 "str x0, [sp, #%d]\n";
        char * zero = calloc(strlen(push_zero) + 128, sizeof(char));
        sprintf(zero, push_zero, z);
        strpush = realloc(strpush, (strlen(strpush) + strlen(zero) + 1) * sizeof(char));
        strcat(strpush, zero);
        free(zero);
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


static int is_string_operand(AST_t* node, dynamic_list_t* list)
{
    if (node->type == STRING_AST) return 1;
    if (node->type == VAR_AST && node->name) {
        for (unsigned int j = 0; j < list->size; j++) {
            AST_t* def = (AST_t*)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name && strcmp(def->name, node->name) == 0)
                return (def->datatype == TYPE_STR);
        }
    }
    return 0;
}

char * assemble_binop(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    if (ast->op == PLUS_TOKEN && is_string_operand(ast->left, list) && is_string_operand(ast->right, list)) {
        char* left_asm = assemble(ast->left, list);
        char* right_asm = assemble(ast->right, list);
        s = realloc(s, strlen(left_asm) + strlen(right_asm) + 128);
        strcat(s, left_asm);
        strcat(s, right_asm);
        int left_off = ast->left->stack_index * -16;
        int right_off = ast->right->stack_index * -16;
        append_load_from_fp(&s, left_off, 0, "BigWord left");
        const char* push = "str x0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(push) + 1);
        strcat(s, push);
        append_load_from_fp(&s, right_off, 0, "BigWord right");
        const char* call = "\n# BigWord\nmov x1, x0\nldr x0, [sp], #16\nbl BigWord\nstr x0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(call) + 1);
        strcat(s, call);
        free(left_asm);
        free(right_asm);
        return s;
    }

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
    /* Byte load/store for bool operands and result */
    const char *left_ldr = (ast->left->datatype == TYPE_BOOL) ? "ldrb" : "ldr";
    const char *right_ldr = (ast->right->datatype == TYPE_BOOL) ? "ldrb" : "ldr";
    const char *result_str = (ast->datatype == TYPE_BOOL) ? "strb" : "str";

    char * op_asm = calloc(1024, sizeof(char));
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
        case SLASH_TOKEN:{
            /* Division by zero check: load divisor, call rt_div_zero_check */
            char div_chk[256];
            if (right_abs > 255)
                sprintf(div_chk, "\n# div-by-zero check\nsub x4, fp, #%d\nldr w0, [x4]\nbl rt_div_zero_check\n", right_abs);
            else
                sprintf(div_chk, "\n# div-by-zero check\nldr w0, [fp, #%d]\nbl rt_div_zero_check\n", right_offset);
            s = realloc(s, strlen(s) + strlen(div_chk) + 1);
            strcat(s, div_chk);
            if (use_large)
                sprintf(op_asm, (char*) assemble_div_large_offset_aarch64, left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, (char*) assemble_div_aarch64, left_offset, right_offset, result_offset);
            break;
        }
        case DEQUALS_TOKEN:
        case NOT_EQUALS_TOKEN:
        case LT_TOKEN:
        case GT_TOKEN:
        case LTE_TOKEN:
        case GTE_TOKEN: {
            const char* cond;
            switch(ast->op) {
                case DEQUALS_TOKEN:     cond = "eq"; break;
                case NOT_EQUALS_TOKEN:  cond = "ne"; break;
                case LT_TOKEN:          cond = "lt"; break;
                case GT_TOKEN:          cond = "gt"; break;
                case LTE_TOKEN:         cond = "le"; break;
                case GTE_TOKEN:         cond = "ge"; break;
                default:                cond = "eq"; break;
            }
            if (use_large) {
                sprintf(op_asm,
                    "# comparison\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n"
                    "sub x4, fp, #%d\n%s w1, [x4]\n"
                    "cmp w0, w1\n"
                    "cset w0, %s\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n",
                    left_abs, left_ldr, right_abs, right_ldr, cond, result_abs, result_str);
            } else {
                sprintf(op_asm,
                    "# comparison\n"
                    "%s w0, [fp, #%d]\n"
                    "%s w1, [fp, #%d]\n"
                    "cmp w0, w1\n"
                    "cset w0, %s\n"
                    "%s w0, [fp, #%d]\n",
                    left_ldr, left_offset, right_ldr, right_offset, cond, result_str, result_offset);
            }
            break;
        }
        case AND_TOKEN:
            if (use_large) {
                sprintf(op_asm,
                    "# logical and\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n"
                    "sub x4, fp, #%d\n%s w1, [x4]\n"
                    "cmp w0, #0\ncset w0, ne\n"
                    "cmp w1, #0\ncset w1, ne\n"
                    "and w0, w0, w1\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n",
                    left_abs, left_ldr, right_abs, right_ldr, result_abs, result_str);
            } else {
                sprintf(op_asm,
                    "# logical and\n"
                    "%s w0, [fp, #%d]\n%s w1, [fp, #%d]\n"
                    "cmp w0, #0\ncset w0, ne\n"
                    "cmp w1, #0\ncset w1, ne\n"
                    "and w0, w0, w1\n"
                    "%s w0, [fp, #%d]\n",
                    left_ldr, left_offset, right_ldr, right_offset, result_str, result_offset);
            }
            break;
        case OR_TOKEN:
            if (use_large) {
                sprintf(op_asm,
                    "# logical or\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n"
                    "sub x4, fp, #%d\n%s w1, [x4]\n"
                    "cmp w0, #0\ncset w0, ne\n"
                    "cmp w1, #0\ncset w1, ne\n"
                    "orr w0, w0, w1\n"
                    "sub x4, fp, #%d\n%s w0, [x4]\n",
                    left_abs, left_ldr, right_abs, right_ldr, result_abs, result_str);
            } else {
                sprintf(op_asm,
                    "# logical or\n"
                    "%s w0, [fp, #%d]\n%s w1, [fp, #%d]\n"
                    "cmp w0, #0\ncset w0, ne\n"
                    "cmp w1, #0\ncset w1, ne\n"
                    "orr w0, w0, w1\n"
                    "%s w0, [fp, #%d]\n",
                    left_ldr, left_offset, right_ldr, right_offset, result_str, result_offset);
            }
            break;
        case BITAND_TOKEN:
            if (use_large) {
                sprintf(op_asm,
                    "# bitwise and\n"
                    "sub x4, fp, #%d\nldr w0, [x4]\n"
                    "sub x4, fp, #%d\nldr w1, [x4]\n"
                    "and w0, w0, w1\n"
                    "sub x4, fp, #%d\nstr w0, [x4]\n",
                    left_abs, right_abs, result_abs);
            } else {
                sprintf(op_asm,
                    "# bitwise and\n"
                    "ldr w0, [fp, #%d]\nldr w1, [fp, #%d]\n"
                    "and w0, w0, w1\n"
                    "str w0, [fp, #%d]\n",
                    left_offset, right_offset, result_offset);
            }
            break;
        case BITOR_TOKEN:
            if (use_large) {
                sprintf(op_asm,
                    "# bitwise or\n"
                    "sub x4, fp, #%d\nldr w0, [x4]\n"
                    "sub x4, fp, #%d\nldr w1, [x4]\n"
                    "orr w0, w0, w1\n"
                    "sub x4, fp, #%d\nstr w0, [x4]\n",
                    left_abs, right_abs, result_abs);
            } else {
                sprintf(op_asm,
                    "# bitwise or\n"
                    "ldr w0, [fp, #%d]\nldr w1, [fp, #%d]\n"
                    "orr w0, w0, w1\n"
                    "str w0, [fp, #%d]\n",
                    left_offset, right_offset, result_offset);
            }
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


char * assemble_slice(AST_t * ast, dynamic_list_t * list)
{
    AST_t* base = (AST_t*)ast->children->items[0];
    AST_t* start_expr = (AST_t*)ast->children->items[1];
    AST_t* end_expr = (AST_t*)ast->children->items[2];
    /* Assemble start/end first so their values are stored before we load them */
    char * s = calloc(1, sizeof(char));
    char* start_asm = assemble(start_expr, list);
    if (start_asm) {
        s = realloc(s, strlen(s) + strlen(start_asm) + 1);
        strcat(s, start_asm);
        free(start_asm);
    }
    char* end_asm = assemble(end_expr, list);
    if (end_asm) {
        s = realloc(s, strlen(s) + strlen(end_asm) + 1);
        strcat(s, end_asm);
        free(end_asm);
    }
    int base_offset = (base->stack_index > 0) ? (base->stack_index * -16) : (ast->int_value * -16);
    if (base_offset == 0 && base->name && ast->stackframe) {
        for (unsigned int i = 0; i < ast->stackframe->stack->size; i++) {
            char* var_name = (char*)ast->stackframe->stack->items[i];
            if (var_name && strcmp(var_name, base->name) == 0) {
                base_offset = (int)(i + 1) * -16;
                break;
            }
        }
    }
    append_load_from_fp(&s, base_offset, 0, "load string for SmolString");
    /* Null string check */
    const char* null_chk = "bl rt_null_str_check\n";
    s = realloc(s, strlen(s) + strlen(null_chk) + 1);
    strcat(s, null_chk);
    const char* push = "str x0, [sp, #-16]!\n";
    s = realloc(s, strlen(s) + strlen(push) + 1);
    strcat(s, push);
    int start_off = start_expr->stack_index * -16;
    int end_off = end_expr->stack_index * -16;
    append_load_w_from_fp(&s, start_off, "load start");
    const char* push2 = "str w0, [sp, #-16]!\n";
    s = realloc(s, strlen(s) + strlen(push2) + 1);
    strcat(s, push2);
    append_load_w_from_fp(&s, end_off, "load end");
    const char* call = "\n# SmolString (slice)\nmov w2, w0\nldr w1, [sp], #16\nldr x0, [sp], #16\nbl SmolString\nstr x0, [sp, #-16]!\n";
    s = realloc(s, strlen(s) + strlen(call) + 1);
    strcat(s, call);
    return s;
}

char * assemble_access(AST_t * ast, dynamic_list_t * list)
{
    /* Assemble the index expression first so its value is stored */
    char * s = calloc(1, sizeof(char));
    if (ast->left) {
        char* idx_asm = assemble(ast->left, list);
        if (idx_asm) {
            s = realloc(s, strlen(s) + strlen(idx_asm) + 1);
            strcat(s, idx_asm);
            free(idx_asm);
        }
    }
    int base_offset = (ast->int_value > 0) ? (ast->int_value * -16) : 0;
    if (base_offset == 0 && ast->name && ast->stackframe) {
        for (unsigned int i = 0; i < ast->stackframe->stack->size; i++) {
            char* var_name = (char*)ast->stackframe->stack->items[i];
            if (var_name && strcmp(var_name, ast->name) == 0) {
                base_offset = (int)(i + 1) * -16;
                break;
            }
        }
    }

    /* Check if base is an array variable */
    int is_array = 0;
    for (unsigned int i = 0; i < list->size; i++) {
        AST_t* def = (AST_t*) list->items[i];
        if (def->type == ASSIGNEMENT_AST && def->name && ast->name &&
            strcmp(def->name, ast->name) == 0 && IS_ARRAY_TYPE(def->datatype)) {
            is_array = 1;
            break;
        }
    }

    if (is_array) {
        /* Array access: base_slot stored at [fp, base_offset], index at idx_offset */
        int idx_offset = ast->left->stack_index * -16;
        /* Find array size for bounds check */
        int arr_size = 0;
        for (unsigned int i = 0; i < list->size; i++) {
            AST_t* def = (AST_t*) list->items[i];
            if (def->type == ASSIGNEMENT_AST && def->name && ast->name &&
                strcmp(def->name, ast->name) == 0 && def->parent &&
                def->parent->type == ARRAY_LITERAL_AST) {
                arr_size = def->parent->int_value;
                break;
            }
        }
        /* Emit bounds check: w0=index, w1=size */
        append_load_w_from_fp(&s, idx_offset, "load array index for bounds check");
        char bounds_chk[128];
        sprintf(bounds_chk, "\n# array bounds check (size=%d)\nmov w1, #%d\nbl rt_array_bounds_check\n", arr_size, arr_size);
        s = realloc(s, strlen(s) + strlen(bounds_chk) + 1);
        strcat(s, bounds_chk);

        append_load_w_from_fp(&s, base_offset, "load array base_slot");
        const char* push = "str w0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(push) + 1);
        strcat(s, push);
        append_load_w_from_fp(&s, idx_offset, "load array index");
        const char* arr_access =
            "\n# array element access\n"
            "ldr w1, [sp], #16\n"
            "add w1, w1, w0\n"
            "mov w3, #16\n"
            "mul w1, w1, w3\n"
            "mov x3, fp\n"
            "sub x3, x3, x1\n"
            "ldr x0, [x3]\n"
            "str x0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(arr_access) + 1);
        strcat(s, arr_access);
    } else {
        /* String access: CharAt */
        append_load_from_fp(&s, base_offset, 0, "load string for CharAt");
        /* Null string check */
        const char* null_chk = "bl rt_null_str_check\n";
        s = realloc(s, strlen(s) + strlen(null_chk) + 1);
        strcat(s, null_chk);
        const char* push = "str x0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(push) + 1);
        strcat(s, push);
        int idx_offset = ast->left->stack_index * -16;
        append_load_w_from_fp(&s, idx_offset, "load index for CharAt");
        const char* call = "\n# CharAt\nmov w1, w0\nldr x0, [sp], #16\nbl CharAt\nstr x0, [sp, #-16]!\n";
        s = realloc(s, strlen(s) + strlen(call) + 1);
        strcat(s, call);
    }
    return s;
}


char * assemble_return(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));
    AST_t * expr = ast->parent;

    /* Unwrap (expr) from parentheses */
    if (expr->type == COMP_AST && expr->children->size == 1)
        expr = (AST_t*) expr->children->items[0];

    if (expr->type == INT_AST)
    {
        char * instr = calloc(64, sizeof(char));
        sprintf(instr, "\nmov w0, #%d\nb return_statement\n", expr->int_value);
        s = realloc(s, strlen(instr) + 1);
        strcat(s, instr);
        free(instr);
    }
    else if (expr->type == VAR_AST || expr->type == BINOP_AST || expr->type == BOOL_AST || expr->type == UNARY_AST)
    {
        char * load = calloc(1, sizeof(char));
        if (expr->datatype == TYPE_BOOL)
            append_load_b_from_fp(&load, expr->stack_index * -16, "return load (bool)");
        else
            append_load_w_from_fp(&load, expr->stack_index * -16, "return load");
        s = realloc(s, strlen(load) + 32);
        strcat(s, load);
        strcat(s, "b return_statement\n");
        free(load);
    }
    else if (expr->type == CALL_AST)
    {
        /* Assemble call; it stores return value on stack. Pop into x0 for return. */
        char * call_asm = assemble_call(expr, list);
        s = realloc(s, strlen(call_asm) + 64);
        strcat(s, call_asm);
        free(call_asm);
        const char * pop_and_branch = "ldr x0, [sp], #16\nb return_statement\n";
        s = realloc(s, strlen(s) + strlen(pop_and_branch) + 1);
        strcat(s, pop_and_branch);
    }
    else
    {
        /* Fallback: assemble and hope value ends up in x0 */
        char * value = assemble(ast->parent, list);
        s = realloc(s, strlen(value) + 32);
        strcat(s, value);
        strcat(s, "\nb return_statement\n");
        free(value);
    }

    return s;
}


char * assemble_function(AST_t* ast, dynamic_list_t* list)
{
    char * name = ast->name;
    int index = ast->stackframe->stack->size * 16;

    size_t s_cap = 65536;
    char * s = calloc(s_cap, sizeof(char));

    sprintf(s, (char*) assembly_function_begin_aarch64, name, index);

    if(ast->stackframe->stack->size)
    {
        const char* sub_template = "\nsub sp, sp, #%d\n";
        char sub[128];
        snprintf(sub, sizeof(sub), sub_template, (1 + ast->stackframe->stack->size) * 16);
        strcat(s, sub);
    }

    AST_t* assembly_value = ast;

    // Set up function parameters - first 8 parameters in registers x0-x7, rest on stack
    /* Use a local list for function body to avoid heap corruption from shared list mutation */
    dynamic_list_t* body_list = init_list(sizeof(struct AST_S*));
    for (unsigned int i = 0; i < assembly_value->children->size; i++)
    {
        AST_t* function_arg = (AST_t*) assembly_value->children->items[i];
        list_enqueue(body_list, function_arg);

        if(i < 8)
        {
            char param_comment[128];
            snprintf(param_comment, sizeof(param_comment), "load parameter %s from x%d", function_arg->name, i);
            char* store_instr = store_x_to_fp_instr(function_arg->stack_index * -16, i, param_comment);
            size_t need = strlen(s) + strlen(store_instr) + 1;
            if (need >= s_cap) {
                s_cap = need + 65536;
                char* new_s = realloc(s, s_cap);
                if (new_s) s = new_s;
            }
            strcat(s, store_instr);
            free(store_instr);
        }
        else
        {
            int stack_offset = 16 + (i - 8) * 16;
            int var_offset = function_arg->stack_index * -16;
            const char* load_template = "\n# load parameter %s from stack\nldr x0, [sp, #%d]\n";
            char param_buf[256];
            int plen = snprintf(param_buf, sizeof(param_buf), load_template, function_arg->name, stack_offset);
            if (plen > 0 && (size_t)(strlen(s) + plen) < s_cap)
                strcat(s, param_buf);
            char* store_instr = store_x_to_fp_instr(var_offset, 0, "store param");
            size_t need = strlen(s) + strlen(store_instr) + 1;
            if (need >= s_cap) {
                s_cap = need + 65536;
                char* new_s = realloc(s, s_cap);
                if (new_s) s = new_s;
            }
            strcat(s, store_instr);
            free(store_instr);
        }
    }

    char* assemble_value_value = assemble(assembly_value->parent, body_list);
    size_t body_len = strlen(assemble_value_value);
    if (strlen(s) + body_len >= s_cap) {
        s_cap = strlen(s) + body_len + 65536;
        char* new_s = realloc(s, s_cap);
        if (new_s) s = new_s;
    }
    strcat(s, assemble_value_value);
    free(assemble_value_value);
    free(body_list->items);
    free(body_list);

    return s;
}

char * assemble_array_literal(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));
    int count = ast->int_value;
    for (int i = 0; i < count; i++) {
        AST_t* elem = (AST_t*)ast->children->items[i];
        char* elem_asm = assemble(elem, list);
        if (elem_asm) {
            s = realloc(s, strlen(s) + strlen(elem_asm) + 1);
            strcat(s, elem_asm);
            free(elem_asm);
        }
    }
    return s;
}


static int if_label_counter = 0;
static int loop_label_counter = 0;

char * assemble_loop_until(AST_t * ast, dynamic_list_t * list);
char * assemble_inc_dec(AST_t * ast, dynamic_list_t * list);

char * assemble_bool(AST_t * ast, dynamic_list_t * list)
{
    int index = ast->stack_index * -16;
    char * s = calloc(256, sizeof(char));
    sprintf(s,
        "# bool (%s)\n"
        "str x0, [sp, #-16]!\n"
        "mov w0, #%d\n",
        ast->int_value ? "Real" : "Fake", ast->int_value);
    append_store_b_to_fp(&s, index, "bool (1 byte)");
    return s;
}

char * assemble_if(AST_t * ast, dynamic_list_t * list)
{
    int id = if_label_counter++;
    char * s = calloc(1, sizeof(char));

    if (ast->left) {
        char* cond_asm = assemble(ast->left, list);
        s = realloc(s, strlen(s) + strlen(cond_asm) + 1);
        strcat(s, cond_asm);
        free(cond_asm);

        int cond_off = ast->left->stack_index * -16;
        if (ast->left->datatype == TYPE_BOOL)
            append_load_b_from_fp(&s, cond_off, "if condition (bool)");
        else
            append_load_w_from_fp(&s, cond_off, "if condition");

        char label_buf[128];
        if (ast->right)
            sprintf(label_buf, "cbz w0, _else_%d\n", id);
        else
            sprintf(label_buf, "cbz w0, _endif_%d\n", id);
        s = realloc(s, strlen(s) + strlen(label_buf) + 1);
        strcat(s, label_buf);
    }

    for (unsigned int i = 0; i < ast->children->size; i++) {
        AST_t* child = (AST_t*)ast->children->items[i];
        char* child_asm = assemble(child, list);
        s = realloc(s, strlen(s) + strlen(child_asm) + 1);
        strcat(s, child_asm);
        free(child_asm);
    }

    if (ast->left) {
        char end_buf[64];
        sprintf(end_buf, "b _endif_%d\n", id);
        s = realloc(s, strlen(s) + strlen(end_buf) + 1);
        strcat(s, end_buf);
    }

    if (ast->right) {
        char else_label[64];
        sprintf(else_label, "_else_%d:\n", id);
        s = realloc(s, strlen(s) + strlen(else_label) + 1);
        strcat(s, else_label);

        char* else_asm = assemble_if(ast->right, list);
        s = realloc(s, strlen(s) + strlen(else_asm) + 1);
        strcat(s, else_asm);
        free(else_asm);
    }

    if (ast->left) {
        char endif_label[64];
        sprintf(endif_label, "_endif_%d:\n", id);
        s = realloc(s, strlen(s) + strlen(endif_label) + 1);
        strcat(s, endif_label);
    }

    return s;
}

char * assemble_unary(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    char* operand_asm = assemble(ast->left, list);
    s = realloc(s, strlen(s) + strlen(operand_asm) + 1);
    strcat(s, operand_asm);
    free(operand_asm);

    if (ast->op == NOT_TOKEN) {
        int lo = ast->left->stack_index * -16;
        int so = ast->stack_index * -16;
        if (ast->left->datatype == TYPE_BOOL)
            append_load_b_from_fp(&s, lo, "logical not load (bool)");
        else
            append_load_w_from_fp(&s, lo, "logical not load");
        const char* not_op = "cmp w0, #0\ncset w0, eq\n";
        s = realloc(s, strlen(s) + strlen(not_op) + 1);
        strcat(s, not_op);
        if (ast->datatype == TYPE_BOOL)
            append_store_b_to_fp(&s, so, "logical not store (bool)");
        else
            append_store_w_to_fp(&s, so, "logical not store");
    } else if (ast->op == BITNOT_TOKEN) {
        append_load_w_from_fp(&s, ast->left->stack_index * -16, "bitwise not load");
        const char* not_op = "mvn w0, w0\n";
        s = realloc(s, strlen(s) + strlen(not_op) + 1);
        strcat(s, not_op);
        append_store_w_to_fp(&s, ast->stack_index * -16, "bitwise not store");
    }
    return s;
}

char * assemble_inc_dec(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));
    int var_offset = ast->left->stack_index * -16;
    int result_offset = ast->stack_index * -16;
    int is_postfix = (ast->int_value != 0);
    int is_inc = (ast->op == PLUS_PLUS_TOKEN);

    append_load_w_from_fp(&s, var_offset, "inc/dec load var");
    if (is_postfix) {
        append_store_w_to_fp(&s, result_offset, "postfix: save old value");
    }
    if (is_inc) {
        const char* add1 = "add w0, w0, #1\n";
        s = realloc(s, strlen(s) + strlen(add1) + 1);
        strcat(s, add1);
    } else {
        const char* sub1 = "sub w0, w0, #1\n";
        s = realloc(s, strlen(s) + strlen(sub1) + 1);
        strcat(s, sub1);
    }
    append_store_w_to_fp(&s, var_offset, "inc/dec store back");
    if (!is_postfix) {
        append_load_w_from_fp(&s, var_offset, "prefix: load new value");
        append_store_w_to_fp(&s, result_offset, "prefix: store result");
    }
    return s;
}

static void loop_append_body(char **out, AST_t *loop_ast, dynamic_list_t *lst)
{
    for (unsigned int i = 0; i < loop_ast->children->size; i++) {
        AST_t* child = (AST_t*)loop_ast->children->items[i];
        char* child_asm = assemble(child, lst);
        *out = realloc(*out, strlen(*out) + strlen(child_asm) + 1);
        strcat(*out, child_asm);
        free(child_asm);
    }
}

char * assemble_loop_until(AST_t * ast, dynamic_list_t * list)
{
    int id = loop_label_counter++;
    char * s = calloc(1, sizeof(char));
    int body_first = ast->int_value;
    int is_for = (ast->left->type == FOR_CLAUSE_AST);

    if (body_first) {
        char body_label[64];
        sprintf(body_label, "_loop_body_%d:\n", id);
        s = realloc(s, strlen(s) + strlen(body_label) + 1);
        strcat(s, body_label);
        loop_append_body(&s, ast, list);
        if (!is_for) {
            char* cond_asm = assemble(ast->left, list);
            s = realloc(s, strlen(s) + strlen(cond_asm) + 1);
            strcat(s, cond_asm);
            free(cond_asm);
            int cond_off = ast->left->stack_index * -16;
            if (ast->left->datatype == TYPE_BOOL)
                append_load_b_from_fp(&s, cond_off, "do-while condition (bool)");
            else
                append_load_w_from_fp(&s, cond_off, "do-while condition");
            char branch[64];
            sprintf(branch, "cbnz w0, _loop_body_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
        } else {
            char branch[64];
            AST_t* for_clause = ast->left;
            AST_t* init = (AST_t*)for_clause->children->items[0];
            AST_t* cond = (AST_t*)for_clause->children->items[1];
            AST_t* step = (AST_t*)for_clause->children->items[2];
            char* init_asm = assemble(init, list);
            s = realloc(s, strlen(s) + strlen(init_asm) + 1);
            strcat(s, init_asm);
            free(init_asm);
            sprintf(body_label, "_loop_cond_%d:\n", id);
            s = realloc(s, strlen(s) + strlen(body_label) + 1);
            strcat(s, body_label);
            char* cond_asm = assemble(cond, list);
            s = realloc(s, strlen(s) + strlen(cond_asm) + 1);
            strcat(s, cond_asm);
            free(cond_asm);
            if (cond->datatype == TYPE_BOOL)
                append_load_b_from_fp(&s, cond->stack_index * -16, "for condition (bool)");
            else
                append_load_w_from_fp(&s, cond->stack_index * -16, "for condition");
            sprintf(branch, "cbz w0, _loop_end_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
            loop_append_body(&s, ast, list);
            char* step_asm = assemble(step, list);
            s = realloc(s, strlen(s) + strlen(step_asm) + 1);
            strcat(s, step_asm);
            free(step_asm);
            sprintf(branch, "b _loop_cond_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
        }
    } else {
        if (!is_for) {
            char cond_label[64];
            sprintf(cond_label, "_loop_cond_%d:\n", id);
            s = realloc(s, strlen(s) + strlen(cond_label) + 1);
            strcat(s, cond_label);
            char* cond_asm = assemble(ast->left, list);
            s = realloc(s, strlen(s) + strlen(cond_asm) + 1);
            strcat(s, cond_asm);
            free(cond_asm);
            if (ast->left->datatype == TYPE_BOOL)
                append_load_b_from_fp(&s, ast->left->stack_index * -16, "while condition (bool)");
            else
                append_load_w_from_fp(&s, ast->left->stack_index * -16, "while condition");
            char branch[64];
            sprintf(branch, "cbz w0, _loop_end_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
            loop_append_body(&s, ast, list);
            sprintf(branch, "b _loop_cond_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
        } else {
            AST_t* for_clause = ast->left;
            AST_t* init = (AST_t*)for_clause->children->items[0];
            AST_t* cond = (AST_t*)for_clause->children->items[1];
            AST_t* step = (AST_t*)for_clause->children->items[2];
            char* init_asm = assemble(init, list);
            s = realloc(s, strlen(s) + strlen(init_asm) + 1);
            strcat(s, init_asm);
            free(init_asm);
            char cond_label[64];
            sprintf(cond_label, "_loop_cond_%d:\n", id);
            s = realloc(s, strlen(s) + strlen(cond_label) + 1);
            strcat(s, cond_label);
            char* cond_asm = assemble(cond, list);
            s = realloc(s, strlen(s) + strlen(cond_asm) + 1);
            strcat(s, cond_asm);
            free(cond_asm);
            if (cond->datatype == TYPE_BOOL)
                append_load_b_from_fp(&s, cond->stack_index * -16, "for condition (bool)");
            else
                append_load_w_from_fp(&s, cond->stack_index * -16, "for condition");
            char branch[64];
            sprintf(branch, "cbz w0, _loop_end_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
            loop_append_body(&s, ast, list);
            char* step_asm = assemble(step, list);
            s = realloc(s, strlen(s) + strlen(step_asm) + 1);
            strcat(s, step_asm);
            free(step_asm);
            sprintf(branch, "b _loop_cond_%d\n", id);
            s = realloc(s, strlen(s) + strlen(branch) + 1);
            strcat(s, branch);
        }
    }
    char end_label[64];
    sprintf(end_label, "_loop_end_%d:\n", id);
    s = realloc(s, strlen(s) + strlen(end_label) + 1);
    strcat(s, end_label);
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
        case SLICE_AST : next = assemble_slice(ast, list); break;
        case ARRAY_LITERAL_AST : next = assemble_array_literal(ast, list); break;
        case RETURN_AST : next = assemble_return(ast, list); break;
        case FUNC_AST : next = assemble_function(ast, list); break;
        case BOOL_AST : next = assemble_bool(ast, list); break;
        case IF_AST : next = assemble_if(ast, list); break;
        case UNARY_AST : next = assemble_unary(ast, list); break;
        case LOOP_UNTIL_AST : next = assemble_loop_until(ast, list); break;
        case INC_DEC_AST : next = assemble_inc_dec(ast, list); break;
        default : {printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type); exit(1);} break;

    }

    value = realloc(value, (strlen(next) +1 ) * sizeof(char));
    strcat(value, next);

    return value;
}
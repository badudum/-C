
#include "include/assembly.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "include/cust.h"
#include "include/types.h"

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
#include "include/assembly/root_ass_x86_64.h"
#include "include/assembly/function_ass_x86_64.h"
#include "include/assembly/int_ass_x86_64.h"
#include "include/assembly/bootstrap_ass_x86_64.h"
#include "include/assembly_emit.h"
#include "include/assembly_target.h"

char * assemble(AST_t * ast, dynamic_list_t * list);

static AST_t * unwrap_comp(AST_t *ast)
{
    if (ast && ast->type == COMP_AST && ast->children && ast->children->size == 1)
        return (AST_t *)ast->children->items[0];
    return ast;
}

static int asm_operand_on_stack(AST_t *ast)
{
    ast = unwrap_comp(ast);
    if (!ast) return 0;
    return ast->type == ACCESS_AST || ast->type == CALL_AST || ast->type == SLICE_AST ||
           ast->type == FIELD_ACCESS_AST;
}

static int asm_str_literal_id = 0;

static char *asm_escape_asciz(const char *src)
{
    size_t len = strlen(src);
    char *out = malloc(len * 4 + 3);
    char *p = out;
    *p++ = '"';
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)src[i];
        switch (c) {
        case '\\': *p++ = '\\'; *p++ = '\\'; break;
        case '"':  *p++ = '\\'; *p++ = '"'; break;
        case '\n': *p++ = '\\'; *p++ = 'n'; break;
        case '\r': *p++ = '\\'; *p++ = 'r'; break;
        case '\t': *p++ = '\\'; *p++ = 't'; break;
        default:
            if (c >= 32 && c < 127)
                *p++ = (char)c;
            else
                p += sprintf(p, "\\x%02x", c);
        }
    }
    *p++ = '"';
    *p = '\0';
    return out;
}

static int asm_arg_needs_assemble(AST_t *arg)
{
    arg = unwrap_comp(arg);
    if (!arg) return 0;
    if (asm_operand_on_stack(arg)) return 1;
    if (arg->type == VAR_AST) return 0;
    return 1;
}

static int cust_fp_offset(int base_stack_index, int field_byte_offset)
{
    return -(base_stack_index * 16) + field_byte_offset;
}

static void append_copy_cust_fields(char **s, int dst_si, int src_si, int cust_id, const char *comment)
{
    cust_type_t *type = cust_get(cust_id);
    if (!type)
        return;
    for (unsigned int i = 0; i < type->fields->size; i++) {
        cust_field_t *f = (cust_field_t *)type->fields->items[i];
        int dst_off = cust_fp_offset(dst_si, f->offset);
        int src_off = cust_fp_offset(src_si, f->offset);
        char cmt[96];
        snprintf(cmt, sizeof(cmt), "%s load %s", comment, f->name);
        asm_append_load_value_from_fp_offset(s, src_off, f->datatype, cmt);
        snprintf(cmt, sizeof(cmt), "%s store %s", comment, f->name);
        asm_append_store_value_to_fp_offset(s, dst_off, f->datatype, cmt);
    }
}

static void append_init_cust_from_literal(char **s, AST_t *init, int dst_si, dynamic_list_t *list)
{
    for (unsigned int i = 0; i < init->children->size; i++) {
        AST_t *entry = (AST_t *)init->children->items[i];
        char *rhs_asm = assemble(entry->parent, list);
        if (rhs_asm) {
            *s = realloc(*s, strlen(*s) + strlen(rhs_asm) + 1);
            strcat(*s, rhs_asm);
            free(rhs_asm);
        }
        AST_t *rhs = unwrap_comp(entry->parent);
        int fp_off = cust_fp_offset(dst_si, entry->int_value);
        if (rhs && asm_operand_on_stack(rhs)) {
            if (entry->datatype == TYPE_INT || entry->datatype == TYPE_BOOL ||
                IS_ARRAY_TYPE(entry->datatype))
                asm_append_pop_expr_word(s, entry->datatype == TYPE_BOOL, "cust init pop");
            else
                asm_append_pop_expr_ptr(s, "cust init pop");
        } else if (rhs && (rhs->type == VAR_AST || rhs->type == INT_AST || rhs->type == BOOL_AST)) {
            int rhs_off = rhs->stack_index * -16;
            if (entry->datatype == TYPE_BOOL)
                asm_append_load_b_from_fp(s, rhs_off, "cust init field load");
            else if (entry->datatype == TYPE_INT || IS_ARRAY_TYPE(entry->datatype))
                asm_append_load_w_from_fp(s, rhs_off, "cust init field load");
            else
                asm_append_load_from_fp(s, rhs_off, 0, "cust init field load");
        } else if (rhs && rhs->type == CUST_INIT_AST) {
            int nested_id = rhs->int_value;
            int temp_base = rhs->stack_index;
            append_init_cust_from_literal(s, rhs, temp_base, list);
            append_copy_cust_fields(s, dst_si, temp_base, nested_id, "nested cust");
            continue;
        }
        char cmt[64];
        snprintf(cmt, sizeof(cmt), "cust init store %s", entry->name ? entry->name : "");
        asm_append_store_value_to_fp_offset(s, fp_off, entry->datatype, cmt);
    }
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
        free(next);
        /* Standalone calls push a return slot; discard it to keep sp aligned. */
        if (child->type == CALL_AST || child->type == DUPE_AST) {
            AST_t *call = child;
            if (call->type != CALL_AST ||
                (strcmp(call->name, "HelloWorld") != 0 &&
                 strcmp(call->name, "HelloWorldLine") != 0))
                asm_append_discard_stack_slot(&value);
        }
    }
    return value;
}


// this is the assembly conversion for assignment occurrences
char * assemble_assignment(AST_t * ast, dynamic_list_t * list)
{
    int id = (ast->stack_index * 16);

    char* s = calloc(1, sizeof(char));

    list_enqueue(list, ast);

    if (ast->left && ast->left->type == FIELD_ACCESS_AST) {
        char *rhs_asm = assemble(ast->parent, list);
        if (rhs_asm) {
            s = realloc(s, strlen(s) + strlen(rhs_asm) + 1);
            strcat(s, rhs_asm);
            free(rhs_asm);
        }
        AST_t *fa = ast->left;
        AST_t *rhs = unwrap_comp(ast->parent);
        int fp_off = cust_fp_offset(fa->id, fa->int_value);
        if (rhs && asm_operand_on_stack(rhs)) {
            if (fa->datatype == TYPE_INT || fa->datatype == TYPE_BOOL ||
                IS_ARRAY_TYPE(fa->datatype))
                asm_append_pop_expr_word(&s, fa->datatype == TYPE_BOOL, "field assign pop");
            else
                asm_append_pop_expr_ptr(&s, "field assign pop");
        } else if (rhs) {
            int rhs_off = rhs->stack_index * -16;
            if (fa->datatype == TYPE_BOOL)
                asm_append_load_b_from_fp(&s, rhs_off, "field assign load");
            else if (fa->datatype == TYPE_INT || IS_ARRAY_TYPE(fa->datatype))
                asm_append_load_w_from_fp(&s, rhs_off, "field assign load");
            else
                asm_append_load_from_fp(&s, rhs_off, 0, "field assign load");
        }
        char cmt[64];
        snprintf(cmt, sizeof(cmt), "field assign %s", fa->name ? fa->name : "");
        asm_append_store_value_to_fp_offset(&s, fp_off, fa->datatype, cmt);
        return s;
    }

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
        asm_append_load_w_from_fp(&s, rhs_offset, "+=/-= load rhs");
        asm_append_mov_word_reg1_from_reg0(&s);
        asm_append_load_w_from_fp(&s, var_offset, "+=/-= load var");
        if (ast->op == PLUS_EQUALS_TOKEN)
            asm_append_add_word_regs(&s);
        else
            asm_append_sub_word_regs(&s);
        asm_append_store_w_to_fp(&s, var_offset, "+=/-= store");
        return s;
    }

    if (ast->parent->type == CALL_AST || ast->parent->type == DUPE_AST)
    {
        /* Return value is in x0 after the call; store to variable and pop stack */
        int var_offset = ast->stack_index * -16;
        if (ast->datatype == TYPE_INT)
            asm_append_store_w_to_fp(&s, var_offset, "assign call store (int)");
        else if (ast->datatype == TYPE_BOOL)
            asm_append_store_b_to_fp(&s, var_offset, "assign call store (bool)");
        else
            asm_append_store_to_fp(&s, var_offset, 0, "assign call store (ptr)");
        asm_append_discard_stack_slot(&s);
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
        sprintf(arr_info, assembly_target_get() == ASSEMBLY_TARGET_X86_64
                ? "\n# assign array (base_slot=%d, count=%d)\nmov eax, %d\n"
                : "\n# assign array (base_slot=%d, count=%d)\nmov w0, #%d\n",
                base_slot, count, base_slot);
        s = realloc(s, strlen(s) + strlen(arr_info) + 1);
        strcat(s, arr_info);
        free(arr_info);
        asm_append_store_w_to_fp(&s, var_offset, "store array base slot");
    }

    else if (IS_CUST_TYPE(ast->datatype))
    {
        int cust_id = CUST_TYPE_ID(ast->datatype);
        AST_t *rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t *)rhs->children->items[0];
        if (rhs->type == CUST_INIT_AST) {
            append_init_cust_from_literal(&s, rhs, ast->stack_index, list);
        } else if (rhs->type == VAR_AST && IS_CUST_TYPE(rhs->datatype)) {
            append_copy_cust_fields(&s, ast->stack_index, rhs->stack_index, cust_id, "cust assign");
        }
    }

    else if(ast->datatype == TYPE_INT || ast->datatype == TYPE_BOOL)
    {
        AST_t* rhs = ast->parent;
        if (rhs->type == COMP_AST && rhs->children->size == 1)
            rhs = (AST_t*) rhs->children->items[0];
        int var_offset = ast->stack_index * -16;
        if (rhs->type == ACCESS_AST) {
            asm_append_pop_expr_word(&s, 0, "assign int from access");
            asm_append_store_w_to_fp(&s, var_offset, "assign int from access");
        } else if (rhs->type == FIELD_ACCESS_AST) {
            asm_append_pop_expr_word(&s, 0, "assign int from field");
            asm_append_store_w_to_fp(&s, var_offset, "assign int from field");
        } else {
            int rhs_offset = rhs->stack_index * -16;
            if (ast->datatype == TYPE_BOOL) {
                asm_append_load_b_from_fp(&s, rhs_offset, "assign (bool) load");
                asm_append_store_b_to_fp(&s, var_offset, "assign (bool) store");
            } else {
                asm_append_load_w_from_fp(&s, rhs_offset, "assign (int) load");
                asm_append_store_w_to_fp(&s, var_offset, "assign (int) store");
            }
        }
    }

    else if(ast->parent->type == BINOP_AST || (ast->parent->type == COMP_AST && ast->parent->children->size == 1))
    {
        AST_t* rhs = ast->parent->type == COMP_AST ? (AST_t*) ast->parent->children->items[0] : ast->parent;
        int rhs_offset = rhs->stack_index * -16;
        int var_offset = ast->stack_index * -16;
        if (ast->datatype == TYPE_STR) {
            asm_append_pop_expr_ptr(&s, "assign str binop");
            asm_append_store_to_fp(&s, var_offset, 0, "assign binop store (str)");
        } else {
            if (ast->datatype == TYPE_BOOL) {
                asm_append_load_b_from_fp(&s, rhs_offset, "assign binop load (bool)");
                asm_append_store_b_to_fp(&s, var_offset, "assign binop store (bool)");
            } else {
                asm_append_load_w_from_fp(&s, rhs_offset, "assign binop load");
                asm_append_store_w_to_fp(&s, var_offset, "assign binop store");
            }
        }
    }

    else if (ast->datatype == TYPE_STR || ast->datatype == TYPE_ADR)
    {
        AST_t* rhs = ast->parent->type == COMP_AST && ast->parent->children->size == 1
            ? (AST_t*) ast->parent->children->items[0] : ast->parent;
        int var_offset = ast->stack_index * -16;
        /* SLICE, ACCESS push result; STRING_AST stores to fp */
        if (rhs->type == SLICE_AST || rhs->type == ACCESS_AST ||
            rhs->type == CALL_AST || rhs->type == BINOP_AST) {
            asm_append_pop_expr_ptr(&s, "assign str/ptr");
        } else {
            int rhs_offset = rhs->stack_index * -16;
            asm_append_load_from_fp(&s, rhs_offset, 0, "assign str/ptr load");
        }
        asm_append_store_to_fp(&s, var_offset, 0, "assign str/ptr store");
    }

    else 
    {
        int var_offset = ast->stack_index * -16;
        asm_append_frag(&s, "\n# assign default\nadd x0, sp, #0\n",
                        "\n# assign default\nmov rax, rsp\n");
        asm_append_store_to_fp(&s, var_offset, 0, "assign default store");
    }
    return s;
}

// making a variable
char * assemble_variable(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));
    char cmt[64];
    snprintf(cmt, sizeof(cmt), "variable (%s)", ast->name ? ast->name : "");
    asm_append_load_from_fp(&s, ast->stack_index * -16, 0, cmt);
    asm_append_push_ptr_from_reg(&s, 0);
    return s;
}

char *assemble_field_access(AST_t *ast, dynamic_list_t *list)
{
    (void)list;
    char *s = calloc(1, sizeof(char));
    int fp_off = cust_fp_offset(ast->id, ast->int_value);
    asm_append_load_value_from_fp_offset(&s, fp_off, ast->datatype, "field access load");
    int result_off = ast->stack_index * -16;
    if (ast->datatype == TYPE_INT || ast->datatype == TYPE_BOOL || IS_ARRAY_TYPE(ast->datatype)) {
        asm_append_push_expr_word(&s);
        if (ast->datatype == TYPE_BOOL)
            asm_append_store_b_to_fp(&s, result_off, "field access temp");
        else
            asm_append_store_w_to_fp(&s, result_off, "field access temp");
    } else {
        asm_append_push_expr_ptr(&s);
        asm_append_store_to_fp(&s, result_off, 0, "field access temp");
    }
    return s;
}

char *assemble_cust_def(AST_t *ast, dynamic_list_t *list)
{
    (void)ast;
    (void)list;
    return calloc(1, sizeof(char));
}

char *assemble_cust_init(AST_t *ast, dynamic_list_t *list)
{
    (void)ast;
    (void)list;
    return calloc(1, sizeof(char));
}


// making a function call
char * assemble_call(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    int is_hello_world = (strcmp(ast->name, "HelloWorld") == 0);
    int is_hello_world_line = (strcmp(ast->name, "HelloWorldLine") == 0);

    unsigned int num_args = ast->parent->children->size;

    // Calculate stack space needed for arguments beyond the first 8
    unsigned int stack_args = (num_args > 8) ? (num_args - 8) : 0;
    unsigned int total_arg_size = stack_args * 16;

    if(stack_args > 0)
        asm_append_reserve_stack_args(&s, total_arg_size);

    if (is_hello_world || is_hello_world_line) {
        for (unsigned int i = 0; i < num_args; i++) {
            AST_t* arg = (AST_t*) ast->parent->children->items[i];
            if (asm_arg_needs_assemble(arg)) {
                char* arg_asm = assemble(arg, list);
                s = realloc(s, (strlen(s) + strlen(arg_asm) + 1) * sizeof(char));
                strcat(s, arg_asm);
                free(arg_asm);
            }

            int need_itos = 0;
            int need_btos = 0;
            int need_adrlo = 0;
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
            else if (arg->type == ACCESS_AST) {
                if (arg->datatype == TYPE_INT)
                    need_itos = 1;
            }
            else if (arg->type == FIELD_ACCESS_AST) {
                if (arg->datatype == TYPE_INT)
                    need_itos = 1;
                else if (arg->datatype == TYPE_BOOL)
                    need_btos = 1;
            }
            else if (arg->type == CALL_AST) {
                if (arg->datatype == TYPE_INT)
                    need_itos = 1;
                else if (arg->datatype == TYPE_BOOL)
                    need_btos = 1;
                else if (arg->datatype == TYPE_ADR)
                    need_adrlo = 1;
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
                        else if (def->datatype == TYPE_ADR)
                            need_adrlo = 1;
                        break;
                    }
                }
            }
            int off = arg->stack_index * -16;
            char cmt[64];
            snprintf(cmt, sizeof(cmt), "HelloWorld arg %d", i);
            int load_from_stack = (arg->type == ACCESS_AST || arg->type == CALL_AST ||
                                   arg->type == SLICE_AST || arg->type == FIELD_ACCESS_AST);
            if (load_from_stack) {
                asm_append_pop_expr_ptr(&s, "HelloWorld arg pop from stack");
            } else if (need_btos) {
                asm_append_load_b_from_fp(&s, off, cmt);
            } else {
                asm_append_load_from_fp(&s, off, 0, cmt);
            }
            if (need_btos) {
                if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
                    asm_append(&s, "mov edi, eax\n");
                    asm_append_call_runtime(&s, "btos", "bool to string");
                    asm_append(&s, "mov rdi, rax\n");
                } else {
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
                }
            } else if (need_adrlo) {
                asm_append_frag(&s, "bl AdrLo\nbl itos\n", "call AdrLo\ncall itos\nmov rdi, rax\n");
            } else if (need_itos) {
                asm_append_frag(&s, "bl itos\n", "mov edi, eax\ncall itos\nmov rdi, rax\n");
            } else if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
                asm_append(&s, "mov rdi, rax\n");
            }
            asm_append_frag(&s, "bl HelloWorld\n", "call HelloWorld\n");
            skip_hello_call:;
        }
        if (is_hello_world_line) {
            asm_append_frag(&s, "\n# HelloWorldLine newline\nbl HelloWorldLine\n",
                            "\n# HelloWorldLine newline\ncall HelloWorldLine\n");
        }
    } else {
        /* Phase 1: evaluate args (array access / calls leave results on expr stack). */
        for (unsigned int i = 0; i < num_args; i++) {
            AST_t *arg = (AST_t *)ast->parent->children->items[i];
            if (asm_arg_needs_assemble(arg)) {
                char *arg_asm = assemble(arg, list);
                s = realloc(s, strlen(s) + strlen(arg_asm) + 1);
                strcat(s, arg_asm);
                free(arg_asm);
            }
        }

        /* Phase 2: load into registers without clobbering earlier args during assembly. */
        for (unsigned int i = 0; i < num_args; i++) {
            AST_t *arg = (AST_t *)ast->parent->children->items[i];
            char cmt[64];
            if (i < 8) {
                snprintf(cmt, sizeof(cmt), "load arg %u into x%u", i, i);
                asm_append_load_call_arg_to_reg(&s, arg, (int)i, list, cmt);
            } else {
                snprintf(cmt, sizeof(cmt), "load arg %u for stack", i);
                asm_append_load_call_arg_to_reg(&s, arg, 0, list, cmt);
                asm_append_store_stack_arg(&s, (int)(i - 8) * 16);
            }
        }

        asm_append_call_runtime(&s, ast->name, ast->name);
    }

    if(stack_args > 0)
        asm_append_cleanup_stack_args(&s, total_arg_size);

    if (!is_hello_world && !is_hello_world_line)
        asm_append_store_call_result(&s);

    return s;
}


char * assemble_int(AST_t * ast, dynamic_list_t * list)
{
    int index = ast->stack_index * -16;
    int abs_offset = index < 0 ? -index : index;
    char *s;
    int x86 = assembly_target_get() == ASSEMBLY_TARGET_X86_64;

    if (x86) {
        if (abs_offset <= 255) {
            s = calloc(assembly_int_x86_64_len + 128, sizeof(char));
            sprintf(s, (char *)assembly_int_x86_64, ast->int_value, abs_offset);
        } else {
            s = calloc(assembly_int_large_offset_x86_64_len + 128, sizeof(char));
            sprintf(s, (char *)assembly_int_large_offset_x86_64, ast->int_value, abs_offset);
        }
    } else if (abs_offset <= 255) {
        s = calloc(assembly_int_aarch64_len + 128, sizeof(char));
        sprintf(s, (char *)assembly_int_aarch64, ast->int_value, index);
    } else {
        s = calloc(assembly_int_large_offset_aarch64_len + 128, sizeof(char));
        sprintf(s, (char *)assembly_int_large_offset_aarch64, ast->int_value, abs_offset);
    }
    return s;
}


char * assemble_string(AST_t * ast, dynamic_list_t * list)
{
    unsigned int string_len = strlen(ast->string_value);
    int index = ast->stack_index * -16;
    int x86 = assembly_target_get() == ASSEMBLY_TARGET_X86_64;

    char *comment_safe = calloc(string_len + 1, sizeof(char));
    for (unsigned int i = 0; i < string_len; i++) {
        char c = ast->string_value[i];
        comment_safe[i] = (c == '\n' || c == '\r' || c == '\t') ? ' ' : c;
    }
    comment_safe[string_len] = '\0';

    if (x86) {
        char label[32];
        char *escaped = asm_escape_asciz(ast->string_value);
        snprintf(label, sizeof(label), "_mc_str_%d", asm_str_literal_id++);
        char *strpush = calloc(strlen(comment_safe) + strlen(escaped) + 256, 1);
        snprintf(strpush, strlen(comment_safe) + strlen(escaped) + 256,
                 "\n# %s\n"
                 ".section .rodata\n"
                 ".align 1\n"
                 "%s:\n"
                 ".asciz %s\n"
                 ".text\n"
                 "lea rax, [rip + %s]\n",
                 comment_safe, label, escaped, label);
        free(comment_safe);
        free(escaped);
        asm_append_store_to_fp(&strpush, index, 0, "store string address");
        return strpush;
    }

    dynamic_list_t * chunks = str_to_hex_list(ast->string_value);
    unsigned int min_bytes = string_len + 1;
    unsigned int numb_bytes = ((min_bytes + 15) / 16) * 16;
    unsigned int byte_counter = (chunks->size - 1) * 8;

    char header[256];
    snprintf(header, sizeof(header), "\n# %s\nsub sp, sp, #%u\n", comment_safe, numb_bytes);
    free(comment_safe);

    char *strpush = calloc(strlen(header) + 1, sizeof(char));
    strcpy(strpush, header);

    for (unsigned int z = chunks->size * 8; z < numb_bytes; z += 8) {
        char zero[64];
        snprintf(zero, sizeof(zero), "mov x0, #0\nstr x0, [sp, #%u]\n", z);
        size_t need = strlen(strpush) + strlen(zero) + 64;
        strpush = realloc(strpush, need);
        strcat(strpush, zero);
    }

    for (unsigned int i = 0; i < chunks->size; i++) {
        char *push_hex = (char *)chunks->items[(chunks->size - i) - 1];
        char push[128];
        snprintf(push, sizeof(push), "ldr x0, =0x%s\nstr x0, [sp, #%u]\n",
                 push_hex, byte_counter);
        size_t need = strlen(strpush) + strlen(push) + 64;
        strpush = realloc(strpush, need);
        strcat(strpush, push);
        free(push_hex);
        if (byte_counter >= 8)
            byte_counter -= 8;
    }

    {
        char tail[32];
        snprintf(tail, sizeof(tail), "\n add x0, sp, #0\n");
        strpush = realloc(strpush, strlen(strpush) + strlen(tail) + 1);
        strcat(strpush, tail);
    }

    asm_append_store_to_fp(&strpush, index, 0, "store string address");
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
    const char *section;
    size_t section_len;
    const char *bootstrap;
    size_t bootstrap_len;

    if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
        if (assembly_os_get() == ASSEMBLY_OS_LINUX) {
            section = assemble_root_x86_64_linux;
            section_len = assemble_root_x86_64_linux_len;
        } else {
            section = assemble_root_x86_64;
            section_len = assemble_root_x86_64_len;
        }
        bootstrap = assembly_bootstrap_x86_64;
        bootstrap_len = assembly_bootstrap_x86_64_len;
    } else {
        section = assemble_root_aarch64;
        section_len = assemble_root_aarch64_len;
        bootstrap = assembly_bootstrap_aarch64;
        bootstrap_len = assembly_bootstrap_aarch64_len;
    }

    char* value = calloc(section_len + 128, sizeof(char));
    sprintf(value, "%s", section);

    char* next = assemble(ast, list);
    value = (char *)realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
    strcat(value, next);

    value = realloc(value, (strlen(value) + bootstrap_len + 1) * sizeof(char));
    strcat(value, bootstrap);

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
        asm_append_load_from_fp(&s, left_off, 0, "BigWord left");
        asm_append_push_expr_ptr(&s);
        asm_append_load_from_fp(&s, right_off, 0, "BigWord right");
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
            asm_append(&s, "\n# BigWord\nmov rbx, rax\n");
            asm_append_pop_ptr_to_reg(&s, 0, "BigWord pop left");
            asm_append(&s, "mov rdi, rax\nmov rsi, rbx\ncall BigWord\n");
            asm_append_push_expr_ptr(&s);
        } else {
            asm_append_frag(&s,
                "\n# BigWord\nmov x1, x0\nldr x0, [sp], #16\nbl BigWord\nstr x0, [sp, #-16]!\n",
                NULL);
        }
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
                sprintf(op_asm, asm_binop_add_template(1), left_abs, right_abs, result_abs);
            else if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                sprintf(op_asm, asm_binop_add_template(0), left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, asm_binop_add_template(0), left_offset, right_offset, result_offset);
            break;
        case MINUS_TOKEN:
            if (use_large)
                sprintf(op_asm, asm_binop_sub_template(1), left_abs, right_abs, result_abs);
            else if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                sprintf(op_asm, asm_binop_sub_template(0), left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, asm_binop_sub_template(0), left_offset, right_offset, result_offset);
            break;
        case ASTERISK_TOKEN:
            if (use_large)
                sprintf(op_asm, asm_binop_mul_template(1), left_abs, right_abs, result_abs);
            else if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                sprintf(op_asm, asm_binop_mul_template(0), left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, asm_binop_mul_template(0), left_offset, right_offset, result_offset);
            break;
        case SLASH_TOKEN:{
            char div_chk[256];
            if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
                if (right_abs > 255)
                    snprintf(div_chk, sizeof(div_chk),
                             "\n# div-by-zero check\nmov rcx, rbp\nsub rcx, %d\nmov edi, [rcx]\n"
                             "call rt_div_zero_check\n", right_abs);
                else
                    snprintf(div_chk, sizeof(div_chk),
                             "\n# div-by-zero check\nmov edi, [rbp+%d]\n"
                             "call rt_div_zero_check\n", right_offset);
            } else if (right_abs > 255)
                snprintf(div_chk, sizeof(div_chk),
                         "\n# div-by-zero check\nsub x4, fp, #%d\nldr w0, [x4]\nbl rt_div_zero_check\n",
                         right_abs);
            else
                snprintf(div_chk, sizeof(div_chk),
                         "\n# div-by-zero check\nldr w0, [fp, #%d]\nbl rt_div_zero_check\n",
                         right_offset);
            s = realloc(s, strlen(s) + strlen(div_chk) + 1);
            strcat(s, div_chk);
            if (use_large)
                sprintf(op_asm, asm_binop_div_template(1), left_abs, right_abs, result_abs);
            else if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                sprintf(op_asm, asm_binop_div_template(0), left_abs, right_abs, result_abs);
            else
                sprintf(op_asm, asm_binop_div_template(0), left_offset, right_offset, result_offset);
            break;
        }
        case DEQUALS_TOKEN:
        case NOT_EQUALS_TOKEN:
        case LT_TOKEN:
        case GT_TOKEN:
        case LTE_TOKEN:
        case GTE_TOKEN: {
            AST_t *left_node = unwrap_comp(ast->left);
            AST_t *right_node = unwrap_comp(ast->right);
            int left_bool = left_node && left_node->datatype == TYPE_BOOL;
            int right_bool = right_node && right_node->datatype == TYPE_BOOL;
            int result_bool = ast->datatype == TYPE_BOOL;
            asm_append_load_binop_operand(&s, ast->left, list, 0, "comparison left");
            asm_append_load_binop_operand(&s, ast->right, list, 1, "comparison right");
            char *cmp_out = calloc(1, 1);
            asm_emit_binop_comparison(&cmp_out, ast->op, use_large,
                left_offset, right_offset, result_offset,
                left_abs, right_abs, result_abs,
                left_bool, right_bool, result_bool);
            strcpy(op_asm, cmp_out);
            free(cmp_out);
            break;
        }
        case AND_TOKEN:
        case OR_TOKEN: {
            int left_bool = ast->left->datatype == TYPE_BOOL;
            int right_bool = ast->right->datatype == TYPE_BOOL;
            int result_bool = ast->datatype == TYPE_BOOL;
            char *logic_out = calloc(1, 1);
            asm_emit_binop_logical(&logic_out, ast->op, use_large,
                left_offset, right_offset, result_offset,
                left_abs, right_abs, result_abs,
                left_bool, right_bool, result_bool);
            strcpy(op_asm, logic_out);
            free(logic_out);
            break;
        }
        case BITAND_TOKEN:
        case BITOR_TOKEN: {
            char *bit_out = calloc(1, 1);
            asm_emit_binop_bitwise(&bit_out, ast->op, use_large,
                left_offset, right_offset, result_offset,
                left_abs, right_abs, result_abs);
            strcpy(op_asm, bit_out);
            free(bit_out);
            break;
        }
        default:
            printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type);
            exit(1);
    }

    s = realloc(s, (strlen(s) + strlen(op_asm) + 1) * sizeof(char));
    strcat(s, op_asm);
    free(op_asm);
    return s;
}

char * assemble_dupe(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    if (ast->left) {
        char* arg_asm = assemble(ast->left, list);
        s = realloc(s, strlen(s) + strlen(arg_asm) + 1);
        strcat(s, arg_asm);
        free(arg_asm);
    }

    asm_append_dupe_fn_load(&s, ast->name);

    if (ast->left) {
        AST_t *arg = ast->left;
        if (arg->type == COMP_AST && arg->children && arg->children->size == 1)
            arg = (AST_t *)arg->children->items[0];
        if (arg->type == ACCESS_AST || arg->type == CALL_AST || arg->type == SLICE_AST) {
            if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
                asm_append_pop_ptr_to_reg(&s, 0, "dupe arg pop");
                asm_append(&s, "mov rsi, rax\n");
            } else {
                asm_append_frag(&s, "\n# dupe arg pop\nldr x1, [sp], #16\n", NULL);
            }
        } else {
            asm_append_load_w_from_fp(&s, arg->stack_index * -16, "dupe arg load");
            if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                asm_append(&s, "movsxd rsi, eax\n");
        }
    }

    asm_append_frag(&s,
        "\nbl rt_dupe_spawn\n# store dupe thread id\nstr x0, [sp, #-16]!\n",
        "\ncall rt_dupe_spawn\n# store dupe thread id\nsub rsp, 16\nmov [rsp], rax\n");
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
    asm_append_load_from_fp(&s, base_offset, 0, "load string for SmolString");
    asm_append_frag(&s, "bl rt_null_str_check\n", "call rt_null_str_check\n");
    asm_append_push_expr_ptr(&s);
    int start_off = start_expr->stack_index * -16;
    int end_off = end_expr->stack_index * -16;
    asm_append_load_w_from_fp(&s, start_off, "load start");
    asm_append_push_expr_word(&s);
    asm_append_load_w_from_fp(&s, end_off, "load end");
    if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
        asm_append(&s, "\n# SmolString (slice)\nmov edx, eax\n");
        asm_append_pop_word_to_reg(&s, 1, 0, "SmolString pop start");
        asm_append_pop_ptr_to_reg(&s, 0, "SmolString pop str");
        asm_append(&s, "mov rdi, rax\nmov esi, ebx\ncall SmolString\n");
        asm_append_push_expr_ptr(&s);
    } else {
        asm_append_frag(&s,
            "\n# SmolString (slice)\nmov w2, w0\nldr w1, [sp], #16\nldr x0, [sp], #16\nbl SmolString\nstr x0, [sp, #-16]!\n",
            NULL);
    }
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
    int is_adr = 0;
    for (unsigned int i = 0; i < list->size; i++) {
        AST_t* def = (AST_t*) list->items[i];
        if (def->type == ASSIGNEMENT_AST && def->name && ast->name &&
            strcmp(def->name, ast->name) == 0) {
            if (IS_ARRAY_TYPE(def->datatype))
                is_array = 1;
            else if (def->datatype == TYPE_ADR)
                is_adr = 1;
            break;
        }
    }

    if (is_adr) {
        int idx_offset = ast->left->stack_index * -16;
        asm_append_load_from_fp(&s, base_offset, 0, "load adr for PeekByte");
        asm_append_frag(&s, "bl rt_heap_adr_check\n", "call rt_heap_adr_check\n");
        asm_append_push_expr_ptr(&s);
        asm_append_load_w_from_fp(&s, idx_offset, "load offset for PeekByte");
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
            asm_append(&s, "\n# PeekByte (adr[offset])\nmov esi, eax\n");
            asm_append_pop_ptr_to_reg(&s, 0, "PeekByte pop adr");
            asm_append(&s, "mov rdi, rax\ncall PeekByte\n");
            asm_append_push_expr_ptr(&s);
        } else {
            asm_append_frag(&s,
                "\n# PeekByte (adr[offset])\nmov w1, w0\nldr x0, [sp], #16\nbl PeekByte\nstr x0, [sp, #-16]!\n",
                NULL);
        }
    } else if (is_array) {
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
        asm_append_load_w_from_fp(&s, idx_offset, "load array index for bounds check");
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
            char bounds_chk[128];
            snprintf(bounds_chk, sizeof(bounds_chk),
                     "\n# array bounds check (size=%d)\nmov edi, eax\nmov esi, %d\ncall rt_array_bounds_check\n",
                     arr_size, arr_size);
            asm_append(&s, bounds_chk);
        } else {
            char bounds_chk[128];
            snprintf(bounds_chk, sizeof(bounds_chk),
                     "\n# array bounds check (size=%d)\nmov w1, #%d\nbl rt_array_bounds_check\n",
                     arr_size, arr_size);
            asm_append(&s, bounds_chk);
        }

        asm_append_load_w_from_fp(&s, base_offset, "load array base_slot");
        asm_append_push_expr_word(&s);
        asm_append_load_w_from_fp(&s, idx_offset, "load array index");
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
            asm_append_pop_word_to_reg(&s, 1, 0, "array access pop base_slot");
            asm_append(&s,
                "\n# array element access\n"
                "movsxd rax, eax\n"
                "add rax, rbx\n"
                "imul rax, rax, 16\n"
                "mov rcx, rbp\n"
                "sub rcx, rax\n"
                "mov rax, [rcx]\n");
            asm_append_push_expr_ptr(&s);
        } else {
            asm_append_frag(&s,
                "\n# array element access\n"
                "ldr w1, [sp], #16\n"
                "add w1, w1, w0\n"
                "mov w3, #16\n"
                "mul w1, w1, w3\n"
                "mov x3, fp\n"
                "sub x3, x3, x1\n"
                "ldr x0, [x3]\n"
                "str x0, [sp, #-16]!\n",
                NULL);
        }
    } else {
        asm_append_load_from_fp(&s, base_offset, 0, "load string for CharAt");
        asm_append_frag(&s, "bl rt_null_str_check\n", "call rt_null_str_check\n");
        asm_append_push_expr_ptr(&s);
        int idx_offset = ast->left->stack_index * -16;
        asm_append_load_w_from_fp(&s, idx_offset, "load index for CharAt");
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
            asm_append(&s, "\n# CharAt\nmov esi, eax\n");
            asm_append_pop_ptr_to_reg(&s, 0, "CharAt pop str");
            asm_append(&s, "mov rdi, rax\ncall CharAt\n");
            asm_append_push_expr_ptr(&s);
        } else {
            asm_append_frag(&s,
                "\n# CharAt\nmov w1, w0\nldr x0, [sp], #16\nbl CharAt\nstr x0, [sp, #-16]!\n",
                NULL);
        }
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
        asm_append_int_return(&s, expr->int_value);
        asm_append_return_epilogue(&s);
    }
    else if (expr->type == VAR_AST || expr->type == BINOP_AST || expr->type == BOOL_AST || expr->type == UNARY_AST)
    {
        char * load = calloc(1, sizeof(char));
        if (expr->datatype == TYPE_BOOL)
            asm_append_load_b_from_fp(&load, expr->stack_index * -16, "return load (bool)");
        else if (expr->datatype == TYPE_STR || expr->datatype == TYPE_ADR)
            asm_append_load_from_fp(&load, expr->stack_index * -16, 0, "return load (ptr/str)");
        else
            asm_append_load_w_from_fp(&load, expr->stack_index * -16, "return load");
        s = realloc(s, strlen(load) + 64 + 1);
        strcat(s, load);
        asm_append_return_epilogue(&s);
        free(load);
    }
    else if (expr->type == CALL_AST)
    {
        /* Assemble call; it stores return value on stack. Pop into x0 for return. */
        char * call_asm = assemble_call(expr, list);
        s = realloc(s, strlen(call_asm) + 64);
        strcat(s, call_asm);
        free(call_asm);
        asm_append_pop_expr_ptr(&s, "return call result");
        asm_append_return_epilogue(&s);
    }
    else
    {
        /* Fallback: assemble and hope value ends up in x0 */
        char * value = assemble(ast->parent, list);
        s = realloc(s, strlen(value) + 64 + 1);
        strcat(s, value);
        asm_append_return_epilogue(&s);
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

    sprintf(s, assembly_target_get() == ASSEMBLY_TARGET_X86_64
                ? (char *)assembly_function_begin_x86_64
                : (char *)assembly_function_begin_aarch64,
            name, index);

    if (ast->stackframe->stack->size) {
        char sub[128];
        if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
            snprintf(sub, sizeof(sub), "\nsub rsp, %d\n",
                     (1 + ast->stackframe->stack->size) * 16);
        else
            snprintf(sub, sizeof(sub), "\nsub sp, sp, #%d\n",
                     (1 + ast->stackframe->stack->size) * 16);
        if (strlen(s) + strlen(sub) >= s_cap) {
            s_cap = strlen(s) + strlen(sub) + 65536;
            char *new_s = realloc(s, s_cap);
            if (new_s) s = new_s;
        }
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

        if (i < 8)
        {
            char param_comment[128];
            snprintf(param_comment, sizeof(param_comment), "load parameter %s", function_arg->name);
            if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
                static const char *regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                char buf[160];
                int var_abs = function_arg->stack_index * 16;
                snprintf(buf, sizeof(buf), "\n# %s\nmov [rbp-%d], %s\n",
                         param_comment, var_abs, regs[i]);
                size_t need = strlen(s) + strlen(buf) + 1;
                if (need >= s_cap) {
                    s_cap = need + 65536;
                    char *new_s = realloc(s, s_cap);
                    if (new_s) s = new_s;
                }
                strcat(s, buf);
            } else {
                char *store_instr = asm_store_to_fp_instr(function_arg->stack_index * -16, i, param_comment);
                size_t need = strlen(s) + strlen(store_instr) + 1;
                if (need >= s_cap) {
                    s_cap = need + 65536;
                    char *new_s = realloc(s, s_cap);
                    if (new_s) s = new_s;
                }
                strcat(s, store_instr);
                free(store_instr);
            }
        }
        else
        {
            int stack_offset = 16 + (i - 8) * 16;
            int var_offset = function_arg->stack_index * -16;
            char param_buf[256];
            if (assembly_target_get() == ASSEMBLY_TARGET_X86_64)
                snprintf(param_buf, sizeof(param_buf),
                         "\n# load parameter %s from stack\nmov rax, [rbp+%d]\n",
                         function_arg->name, stack_offset);
            else
                snprintf(param_buf, sizeof(param_buf),
                         "\n# load parameter %s from stack\nldr x0, [sp, #%d]\n",
                         function_arg->name, stack_offset);
            if ((size_t)strlen(s) + strlen(param_buf) < s_cap)
                strcat(s, param_buf);
            char *store_instr = asm_store_to_fp_instr(var_offset, 0, "store param");
            size_t need = strlen(s) + strlen(store_instr) + 1;
            if (need >= s_cap) {
                s_cap = need + 65536;
                char *new_s = realloc(s, s_cap);
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
    if (assembly_target_get() == ASSEMBLY_TARGET_X86_64) {
        sprintf(s, "# bool (%s)\nsub rsp, 16\nmov eax, %d\nmov byte ptr [rsp], al\n",
                ast->int_value ? "Real" : "Fake", ast->int_value);
    } else {
        sprintf(s,
            "# bool (%s)\n"
            "str x0, [sp, #-16]!\n"
            "mov w0, #%d\n",
            ast->int_value ? "Real" : "Fake", ast->int_value);
    }
    asm_append_store_b_to_fp(&s, index, "bool (1 byte)");
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
            asm_append_load_b_from_fp(&s, cond_off, "if condition (bool)");
        else
            asm_append_load_w_from_fp(&s, cond_off, "if condition");

        char label_buf[128];
        if (ast->right)
            snprintf(label_buf, sizeof(label_buf), "_else_%d", id);
        else
            snprintf(label_buf, sizeof(label_buf), "_endif_%d", id);
        asm_append_jz_word(&s, label_buf);
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
        snprintf(end_buf, sizeof(end_buf), "_endif_%d", id);
        asm_append_jmp(&s, end_buf);
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
            asm_append_load_b_from_fp(&s, lo, "logical not load (bool)");
        else
            asm_append_load_w_from_fp(&s, lo, "logical not load");
        asm_append_logical_not_word(&s);
        if (ast->datatype == TYPE_BOOL)
            asm_append_store_b_to_fp(&s, so, "logical not store (bool)");
        else
            asm_append_store_w_to_fp(&s, so, "logical not store");
    } else if (ast->op == BITNOT_TOKEN) {
        asm_append_load_w_from_fp(&s, ast->left->stack_index * -16, "bitwise not load");
        asm_append_bitwise_not_word(&s);
        asm_append_store_w_to_fp(&s, ast->stack_index * -16, "bitwise not store");
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

    asm_append_load_w_from_fp(&s, var_offset, "inc/dec load var");
    if (is_postfix) {
        asm_append_store_w_to_fp(&s, result_offset, "postfix: save old value");
    }
    if (is_inc) {
        asm_append_add_word_imm(&s, 1);
    } else {
        asm_append_sub_word_imm(&s, 1);
    }
    asm_append_store_w_to_fp(&s, var_offset, "inc/dec store back");
    if (!is_postfix) {
        asm_append_load_w_from_fp(&s, var_offset, "prefix: load new value");
        asm_append_store_w_to_fp(&s, result_offset, "prefix: store result");
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
                asm_append_load_b_from_fp(&s, cond_off, "do-while condition (bool)");
            else
                asm_append_load_w_from_fp(&s, cond_off, "do-while condition");
            char branch[64];
            snprintf(branch, sizeof(branch), "_loop_body_%d", id);
            asm_append_jnz_word(&s, branch);
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
                asm_append_load_b_from_fp(&s, cond->stack_index * -16, "for condition (bool)");
            else
                asm_append_load_w_from_fp(&s, cond->stack_index * -16, "for condition");
            snprintf(branch, sizeof(branch), "_loop_end_%d", id);
            asm_append_jz_word(&s, branch);
            loop_append_body(&s, ast, list);
            char* step_asm = assemble(step, list);
            s = realloc(s, strlen(s) + strlen(step_asm) + 1);
            strcat(s, step_asm);
            free(step_asm);
            snprintf(branch, sizeof(branch), "_loop_cond_%d", id);
            asm_append_jmp(&s, branch);
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
                asm_append_load_b_from_fp(&s, ast->left->stack_index * -16, "while condition (bool)");
            else
                asm_append_load_w_from_fp(&s, ast->left->stack_index * -16, "while condition");
            char branch[64];
            snprintf(branch, sizeof(branch), "_loop_end_%d", id);
            asm_append_jz_word(&s, branch);
            loop_append_body(&s, ast, list);
            snprintf(branch, sizeof(branch), "_loop_cond_%d", id);
            asm_append_jmp(&s, branch);
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
                asm_append_load_b_from_fp(&s, cond->stack_index * -16, "for condition (bool)");
            else
                asm_append_load_w_from_fp(&s, cond->stack_index * -16, "for condition");
            char branch[64];
            snprintf(branch, sizeof(branch), "_loop_end_%d", id);
            asm_append_jz_word(&s, branch);
            loop_append_body(&s, ast, list);
            char* step_asm = assemble(step, list);
            s = realloc(s, strlen(s) + strlen(step_asm) + 1);
            strcat(s, step_asm);
            free(step_asm);
            snprintf(branch, sizeof(branch), "_loop_cond_%d", id);
            asm_append_jmp(&s, branch);
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
        case DUPE_AST : next = assemble_dupe(ast, list); break;
        case TYPE_SIZE_AST : next = assemble_int(ast, list); break;
        case CUST_DEF_AST : next = assemble_cust_def(ast, list); break;
        case CUST_INIT_AST : next = assemble_cust_init(ast, list); break;
        case FIELD_ACCESS_AST : next = assemble_field_access(ast, list); break;
        default : {printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type); exit(1);} break;

    }

    value = realloc(value, (strlen(next) +1 ) * sizeof(char));
    strcat(value, next);

    return value;
}
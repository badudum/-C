#include "include/assembly_emit.h"
#include "include/assembly_target.h"
#include "include/types.h"
#include "include/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/assembly/add_ass.h"
#include "include/assembly/sub_ass.h"
#include "include/assembly/mul_ass.h"
#include "include/assembly/div_ass.h"
#include "include/assembly/add_ass_x86_64.h"
#include "include/assembly/sub_ass_x86_64.h"
#include "include/assembly/mul_ass_x86_64.h"
#include "include/assembly/div_ass_x86_64.h"
#include "include/token.h"

static int is_x86(void)
{
    return assembly_target_get() == ASSEMBLY_TARGET_X86_64;
}

void asm_append(char **s, const char *fragment)
{
    if (!fragment || !*fragment)
        return;
    *s = realloc(*s, strlen(*s) + strlen(fragment) + 1);
    strcat(*s, fragment);
}

void asm_append_return_epilogue(char **s)
{
    if (is_x86()) {
        asm_append(s, "mov rsp, rbp\npop rbp\nret\n");
    } else {
        asm_append(s, "mov sp, x29\nldp x29, x30, [sp], #16\nret\n");
    }
}

void asm_append_load_from_fp(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        static const char *regs64[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *r = regs64[reg < 8 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmov %s, [rbp-%d]\n", comment, r, abs_off);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, [rcx]\n",
                     comment, abs_off, r);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nldr x%d, [fp, #%d]\n", comment, reg, offset);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nldr x%d, [x4]\n",
                     comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_load_w_from_fp_reg(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        static const char *regs32[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d"};
        const char *r = regs32[reg < 6 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmov %s, dword ptr [rbp-%d]\n", comment, r, abs_off);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, dword ptr [rcx]\n",
                     comment, abs_off, r);
    } else if (abs_off <= 255) {
        snprintf(instr, sizeof(instr), "\n# %s\nldr w%d, [fp, #%d]\n", comment, reg, offset);
    } else {
        snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nldr w%d, [x4]\n",
                 comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_load_w_from_fp(char **s, int offset, const char *comment)
{
    asm_append_load_w_from_fp_reg(s, offset, 0, comment);
}

void asm_append_store_w_to_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmov [rbp-%d], eax\n", comment, abs_off);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov [rcx], eax\n",
                     comment, abs_off);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nstr w0, [fp, #%d]\n", comment, offset);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nstr w0, [x4]\n",
                     comment, abs_off);
    }
    asm_append(s, instr);
}

void asm_append_load_b_from_fp_reg(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        static const char *regs32[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d"};
        const char *r = regs32[reg < 6 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmovzx %s, byte ptr [rbp-%d]\n", comment, r, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmovzx %s, byte ptr [rcx]\n",
                     comment, abs_off, r);
    } else if (abs_off <= 255) {
        snprintf(instr, sizeof(instr), "\n# %s\nldrb w%d, [fp, #%d]\n", comment, reg, offset);
    } else {
        snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nldrb w%d, [x4]\n",
                 comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_load_b_from_fp(char **s, int offset, const char *comment)
{
    asm_append_load_b_from_fp_reg(s, offset, 0, comment);
}

void asm_append_store_b_to_fp(char **s, int offset, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmov byte ptr [rbp-%d], al\n", comment, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov byte ptr [rcx], al\n",
                     comment, abs_off);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nstrb w0, [fp, #%d]\n", comment, offset);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nstrb w0, [x4]\n",
                     comment, abs_off);
    }
    asm_append(s, instr);
}

void asm_append_store_to_fp(char **s, int offset, int reg, const char *comment)
{
    int abs_off = offset < 0 ? -offset : offset;
    char instr[256];
    if (is_x86()) {
        static const char *regs64[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *r = regs64[reg < 8 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nmov [rbp-%d], %s\n", comment, abs_off, r);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov [rcx], %s\n",
                     comment, abs_off, r);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr), "\n# %s\nstr x%d, [fp, #%d]\n", comment, reg, offset);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nsub x4, fp, #%d\nstr x%d, [x4]\n",
                     comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_store_value_to_fp_offset(char **s, int fp_offset, int dt, const char *comment)
{
    if (dt == TYPE_BOOL)
        asm_append_store_b_to_fp(s, fp_offset, comment);
    else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt))
        asm_append_store_w_to_fp(s, fp_offset, comment);
    else
        asm_append_store_to_fp(s, fp_offset, 0, comment);
}

void asm_append_load_value_from_fp_offset(char **s, int fp_offset, int dt, const char *comment)
{
    if (dt == TYPE_BOOL)
        asm_append_load_b_from_fp(s, fp_offset, comment);
    else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt))
        asm_append_load_w_from_fp(s, fp_offset, comment);
    else
        asm_append_load_from_fp(s, fp_offset, 0, comment);
}

char *asm_store_to_fp_instr(int offset, int reg, const char *comment)
{
    char buf[256];
    int abs_off = offset < 0 ? -offset : offset;
    if (is_x86()) {
        static const char *regs64[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *r = regs64[reg < 8 ? reg : 0];
        if (abs_off <= 255)
            snprintf(buf, sizeof(buf), "\n# %s\nmov [rbp-%d], %s\n", comment, abs_off, r);
        else
            snprintf(buf, sizeof(buf), "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov [rcx], %s\n",
                     comment, abs_off, r);
    } else {
        if (abs_off <= 255)
            snprintf(buf, sizeof(buf), "\n# %s\nstr x%d, [fp, #%d]\n", comment, reg, offset);
        else
            snprintf(buf, sizeof(buf), "\n# %s\nsub x4, fp, #%d\nstr x%d, [x4]\n",
                     comment, abs_off, reg);
    }
    return strdup(buf);
}

static int asm_arg_datatype(AST_t *arg, dynamic_list_t *list);
static int asm_operand_on_stack(AST_t *ast);
static AST_t *unwrap_comp(AST_t *ast);

int asm_arg_needs_cust_receiver(AST_t *arg, dynamic_list_t *list)
{
    arg = unwrap_comp(arg);
    if (!arg)
        return 0;
    if (IS_CUST_TYPE(arg->datatype))
        return 1;
    if (IS_HEAP_CUST_VAR(arg->datatype, arg->int_value))
        return 1;
    if (arg->type == VAR_AST && arg->name) {
        for (unsigned int j = 0; j < list->size; j++) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, arg->name) == 0) {
                if (IS_CUST_TYPE(def->datatype))
                    return 1;
                if (IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                    return 1;
            }
        }
    }
    return 0;
}

static AST_t *unwrap_comp(AST_t *ast)
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

static int asm_arg_datatype(AST_t *arg, dynamic_list_t *list)
{
    arg = unwrap_comp(arg);
    if (!arg) return TYPE_INT;
    if (arg->type == INT_AST) return TYPE_INT;
    if (arg->type == BOOL_AST) return TYPE_BOOL;
    if (arg->type == STRING_AST) return TYPE_STR;
    if (arg->datatype) return arg->datatype;
    if (arg->type == VAR_AST && arg->name) {
        for (unsigned int j = 0; j < list->size; j++) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, arg->name) == 0)
                return def->datatype;
        }
    }
    if (arg->type == BINOP_AST) {
        if (arg->op == DEQUALS_TOKEN || arg->op == NOT_EQUALS_TOKEN ||
            arg->op == LT_TOKEN || arg->op == GT_TOKEN ||
            arg->op == LTE_TOKEN || arg->op == GTE_TOKEN ||
            arg->op == AND_TOKEN || arg->op == OR_TOKEN)
            return TYPE_BOOL;
        return TYPE_INT;
    }
    if (arg->type == UNARY_AST && arg->op == NOT_TOKEN)
        return TYPE_BOOL;
    return TYPE_INT;
}

static const char *x86_arg_reg(int index)
{
    static const char *regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    if (index < 0 || index > 5)
        return "rax";
    return regs[index];
}

static const char *x86_arg_reg32(int index)
{
    static const char *regs[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    if (index < 0 || index > 5)
        return "eax";
    return regs[index];
}

void asm_append_load_call_arg_to_reg(char **s, AST_t *arg, int reg,
                                     dynamic_list_t *list, const char *comment)
{
    arg = unwrap_comp(arg);
    int dt = asm_arg_datatype(arg, list);
    if (is_x86()) {
        const char *dest64 = x86_arg_reg(reg);
        const char *dest32 = x86_arg_reg32(reg);
        char instr[256];
        if (asm_operand_on_stack(arg)) {
            if (dt == TYPE_BOOL)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmovzx %s, byte ptr [rsp]\nadd rsp, 16\n", comment, dest32);
            else if (dt == TYPE_INT)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov %s, dword ptr [rsp]\nadd rsp, 16\n", comment, dest32);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov %s, [rsp]\nadd rsp, 16\n", comment, dest64);
            asm_append(s, instr);
            return;
        }
        int off = arg->stack_index * -16;
        int abs_off = off < 0 ? -off : off;
        if (dt == TYPE_BOOL) {
            if (abs_off <= 255)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmovzx %s, byte ptr [rbp-%d]\n", comment, dest32, abs_off);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmovzx %s, byte ptr [rcx]\n",
                         comment, abs_off, dest32);
        } else if (dt == TYPE_INT) {
            if (abs_off <= 255)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov %s, dword ptr [rbp-%d]\n", comment, dest32, abs_off);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, dword ptr [rcx]\n",
                         comment, abs_off, dest32);
        } else {
            if (abs_off <= 255)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov %s, [rbp-%d]\n", comment, dest64, abs_off);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, [rcx]\n",
                         comment, abs_off, dest64);
        }
        asm_append(s, instr);
        return;
    }
    if (asm_operand_on_stack(arg)) {
        char instr[128];
        if (dt == TYPE_BOOL)
            snprintf(instr, sizeof(instr), "\n# %s\nldrb w%d, [sp], #16\n", comment, reg);
        else if (dt == TYPE_INT)
            snprintf(instr, sizeof(instr), "\n# %s\nldr w%d, [sp], #16\n", comment, reg);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nldr x%d, [sp], #16\n", comment, reg);
        asm_append(s, instr);
        return;
    }
    int off = arg->stack_index * -16;
    if (dt == TYPE_BOOL) {
        asm_append_load_b_from_fp(s, off, comment);
        if (reg != 0) {
            char mov[48];
            snprintf(mov, sizeof(mov), "mov w%d, w0\n", reg);
            asm_append(s, mov);
        }
    } else if (dt == TYPE_INT) {
        asm_append_load_from_fp(s, off, reg, comment);
    } else {
        asm_append_load_from_fp(s, off, reg, comment);
    }
}

void asm_append_load_cust_receiver_to_reg(char **s, AST_t *arg, int reg, const char *comment)
{
    if (IS_HEAP_CUST_VAR(arg->datatype, arg->int_value)) {
        int off = arg->stack_index * -16;
        asm_append_load_from_fp(s, off, reg, comment);
        return;
    }
    int abs_off = arg->stack_index * 16;
    char instr[256];
    if (is_x86()) {
        static const char *regs[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *dest = regs[reg < 8 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nlea %s, [rbp-%d]\n", comment, dest, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, rcx\n",
                     comment, abs_off, dest);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nsub x%d, fp, #%d\n", comment, reg, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nsub x4, fp, #%d\nmov x%d, x4\n", comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_load_cust_field_receiver_to_reg(char **s, int base_stack_index,
                                                int field_byte_offset, int reg,
                                                const char *comment)
{
    int abs_off = base_stack_index * 16 - field_byte_offset;
    char instr[256];
    if (is_x86()) {
        static const char *regs[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *dest = regs[reg < 8 ? reg : 0];
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nlea %s, [rbp-%d]\n", comment, dest, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, rcx\n",
                     comment, abs_off, dest);
    } else {
        if (abs_off <= 255)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nsub x%d, fp, #%d\n", comment, reg, abs_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nsub x4, fp, #%d\nmov x%d, x4\n", comment, abs_off, reg);
    }
    asm_append(s, instr);
}

void asm_append_load_heap_cust_field_receiver_to_reg(char **s, int base_stack_index,
                                                     int field_byte_offset, int reg,
                                                     const char *comment)
{
    int base_off = base_stack_index * -16;
    char instr[256];
    if (is_x86()) {
        static const char *regs[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        const char *dest = regs[reg < 8 ? reg : 0];
        int abs_base = base_stack_index * 16;
        snprintf(instr, sizeof(instr),
                 "\n# %s\nmov rcx, [rbp-%d]\nlea %s, [rcx+%d]\n",
                 comment, abs_base, dest, field_byte_offset);
    } else {
        if (field_byte_offset == 0)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x%d, [fp, #%d]\n", comment, reg, base_off);
        else
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nadd x%d, x1, #%d\n",
                     comment, base_off, reg, field_byte_offset);
    }
    asm_append(s, instr);
}

void asm_append_heap_field_load(char **s, int base_stack_index, int byte_offset,
                                int dt, const char *comment)
{
    int base_off = base_stack_index * -16;
    int abs_base = base_off < 0 ? -base_off : base_off;
    char instr[384];
    if (is_x86()) {
        if (dt == TYPE_BOOL) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmovzx eax, byte ptr [rcx+%d]\n",
                     comment, abs_base, byte_offset);
        } else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt)) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmov eax, dword ptr [rcx+%d]\n",
                     comment, abs_base, byte_offset);
        } else {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmov rax, [rcx+%d]\n",
                     comment, abs_base, byte_offset);
        }
    } else {
        if (dt == TYPE_BOOL) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nldrb w0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        } else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt)) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nldr w0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        } else {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nldr x0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        }
    }
    asm_append(s, instr);
}

void asm_append_heap_field_store(char **s, int base_stack_index, int byte_offset,
                                 int dt, const char *comment)
{
    int base_off = base_stack_index * -16;
    int abs_base = base_off < 0 ? -base_off : base_off;
    char instr[384];
    if (is_x86()) {
        if (dt == TYPE_BOOL) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmov byte ptr [rcx+%d], al\n",
                     comment, abs_base, byte_offset);
        } else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt)) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmov dword ptr [rcx+%d], eax\n",
                     comment, abs_base, byte_offset);
        } else {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmov rcx, [rbp-%d]\nmov [rcx+%d], rax\n",
                     comment, abs_base, byte_offset);
        }
    } else {
        if (dt == TYPE_BOOL) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nstrb w0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        } else if (dt == TYPE_INT || IS_ARRAY_TYPE(dt)) {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nstr w0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        } else {
            snprintf(instr, sizeof(instr),
                     "\n# %s\nldr x1, [fp, #%d]\nstr x0, [x1, #%d]\n",
                     comment, base_off, byte_offset);
        }
    }
    asm_append(s, instr);
}

void asm_append_pop_ptr_to_reg(char **s, int reg, const char *comment)
{
    char instr[128];
    if (is_x86()) {
        static const char *regs[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        snprintf(instr, sizeof(instr), "\n# %s\nmov %s, [rsp]\nadd rsp, 16\n",
                 comment, regs[reg < 8 ? reg : 0]);
    } else {
        snprintf(instr, sizeof(instr), "\n# %s\nldr x%d, [sp], #16\n", comment, reg);
    }
    asm_append(s, instr);
}

void asm_append_pop_word_to_reg(char **s, int reg, int is_bool, const char *comment)
{
    char instr[128];
    if (is_x86()) {
        static const char *regs[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d", "r10d", "r11d"};
        if (is_bool)
            snprintf(instr, sizeof(instr),
                     "\n# %s\nmovzx %s, byte ptr [rsp]\nadd rsp, 16\n",
                     comment, regs[reg < 8 ? reg : 0]);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nmov %s, [rsp]\nadd rsp, 16\n",
                     comment, regs[reg < 8 ? reg : 0]);
    } else {
        if (is_bool)
            snprintf(instr, sizeof(instr), "\n# %s\nldrb w%d, [sp], #16\n", comment, reg);
        else
            snprintf(instr, sizeof(instr), "\n# %s\nldr w%d, [sp], #16\n", comment, reg);
    }
    asm_append(s, instr);
}

void asm_append_push_ptr_from_reg(char **s, int reg)
{
    if (is_x86()) {
        static const char *regs[] = {"rax", "rbx", "rcx", "rdx", "r8", "r9", "r10", "r11"};
        char instr[64];
        snprintf(instr, sizeof(instr), "sub rsp, 16\nmov [rsp], %s\n", regs[reg < 8 ? reg : 0]);
        asm_append(s, instr);
    } else {
        char instr[64];
        snprintf(instr, sizeof(instr), "str x%d, [sp, #-16]!\n", reg);
        asm_append(s, instr);
    }
}

void asm_append_push_word_from_reg(char **s, int reg)
{
    if (is_x86()) {
        static const char *regs[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d", "r10d", "r11d"};
        char instr[64];
        snprintf(instr, sizeof(instr), "sub rsp, 16\nmov [rsp], %s\n", regs[reg < 8 ? reg : 0]);
        asm_append(s, instr);
    } else {
        char instr[64];
        snprintf(instr, sizeof(instr), "str w%d, [sp, #-16]!\n", reg);
        asm_append(s, instr);
    }
}

void asm_append_discard_stack_slot(char **s)
{
    asm_append(s, is_x86() ? "\n# discard standalone call result\nadd rsp, 16\n"
                           : "\n# discard standalone call result\nadd sp, sp, #16\n");
}

void asm_append_store_call_result(char **s)
{
    asm_append(s, is_x86() ? "\n# store return value\nsub rsp, 16\nmov [rsp], rax\n"
                           : "\n# store return value\nstr x0, [sp, #-16]!\n");
}

void asm_append_call(char **s, const char *name, const char *comment)
{
    char buf[256];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\n# %s\ncall %s\n", comment, name);
    else
        snprintf(buf, sizeof(buf), "\n# %s\nbl %s\n", comment, name);
    asm_append(s, buf);
}

void asm_append_reserve_stack_args(char **s, unsigned int stack_bytes)
{
    if (stack_bytes == 0)
        return;
    char buf[128];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\n# reserve stack arguments\nsub rsp, %u\n", stack_bytes);
    else
        snprintf(buf, sizeof(buf), "\n# reserve stack arguments\nsub sp, sp, #%u\n", stack_bytes);
    asm_append(s, buf);
}

void asm_append_cleanup_stack_args(char **s, unsigned int stack_bytes)
{
    if (stack_bytes == 0)
        return;
    char buf[128];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\n# cleanup stack arguments\nadd rsp, %u\n", stack_bytes);
    else
        snprintf(buf, sizeof(buf), "\n# cleanup stack arguments\nadd sp, sp, #%u\n", stack_bytes);
    asm_append(s, buf);
}

void asm_append_store_stack_arg(char **s, int stack_offset)
{
    char buf[128];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\n# store arg on stack\nmov [rsp+%d], rax\n", stack_offset);
    else
        snprintf(buf, sizeof(buf), "\n# store arg on stack\nstr x0, [sp, #%d]\n", stack_offset);
    asm_append(s, buf);
}

void asm_append_dupe_fn_load(char **s, const char *name)
{
    char buf[256];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\n# dupe spawn %s\nlea rdi, [rip + %s]\n", name, name);
    else
        snprintf(buf, sizeof(buf),
                 "\n# dupe spawn %s\nadrp x0, %s@PAGE\nadd x0, x0, %s@PAGEOFF\n",
                 name, name, name);
    asm_append(s, buf);
}

void asm_append_mov_word_reg(char **s, int dst, int src)
{
    char buf[32];
    if (is_x86()) {
        static const char *regs[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d", "r10d", "r11d"};
        snprintf(buf, sizeof(buf), "mov %s, %s\n", regs[dst < 8 ? dst : 0], regs[src < 8 ? src : 0]);
    } else {
        snprintf(buf, sizeof(buf), "mov w%d, w%d\n", dst, src);
    }
    asm_append(s, buf);
}

void asm_append_int_return(char **s, int value)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\nmov eax, %d\n", value);
    else
        snprintf(buf, sizeof(buf), "\nmov w0, #%d\n", value);
    asm_append(s, buf);
}

void asm_append_branch_if_zero(char **s, const char *label)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "test eax, eax\njz %s\n", label);
    else
        snprintf(buf, sizeof(buf), "cbz w0, %s\n", label);
    asm_append(s, buf);
}

void asm_append_branch_if_nonzero(char **s, const char *label)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "test eax, eax\njnz %s\n", label);
    else
        snprintf(buf, sizeof(buf), "cbnz w0, %s\n", label);
    asm_append(s, buf);
}

void asm_append_logical_not_word(char **s)
{
    asm_append(s, is_x86() ? "cmp eax, 0\nsete al\nmovzx eax, al\n"
                           : "cmp w0, #0\ncset w0, eq\n");
}

void asm_append_bitwise_not_word(char **s)
{
    asm_append(s, is_x86() ? "not eax\n" : "mvn w0, w0\n");
}

void asm_append_add_word_imm(char **s, int imm)
{
    char buf[32];
    if (is_x86())
        snprintf(buf, sizeof(buf), "add eax, %d\n", imm);
    else
        snprintf(buf, sizeof(buf), "add w0, w0, #%d\n", imm);
    asm_append(s, buf);
}

void asm_append_sub_word_imm(char **s, int imm)
{
    char buf[32];
    if (is_x86())
        snprintf(buf, sizeof(buf), "sub eax, %d\n", imm);
    else
        snprintf(buf, sizeof(buf), "sub w0, w0, #%d\n", imm);
    asm_append(s, buf);
}

void asm_append_frag(char **s, const char *aarch64, const char *x86_64)
{
    asm_append(s, is_x86() ? x86_64 : aarch64);
}

const char *asm_cmp_set_suffix(int op)
{
    if (is_x86()) {
        switch (op) {
            case DEQUALS_TOKEN: return "e";
            case NOT_EQUALS_TOKEN: return "ne";
            case LT_TOKEN: return "l";
            case GT_TOKEN: return "g";
            case LTE_TOKEN: return "le";
            case GTE_TOKEN: return "ge";
            default: return "e";
        }
    }
    switch (op) {
        case DEQUALS_TOKEN: return "eq";
        case NOT_EQUALS_TOKEN: return "ne";
        case LT_TOKEN: return "lt";
        case GT_TOKEN: return "gt";
        case LTE_TOKEN: return "le";
        case GTE_TOKEN: return "ge";
        default: return "eq";
    }
}

const char *asm_binop_add_template(int use_large)
{
    if (is_x86())
        return use_large ? assemble_add_large_offset_x86_64 : assemble_add_x86_64;
    return use_large ? assemble_add_large_offset_aarch64 : assemble_add_aarch64;
}

const char *asm_binop_sub_template(int use_large)
{
    if (is_x86())
        return use_large ? assemble_sub_large_offset_x86_64 : assemble_sub_x86_64;
    return use_large ? assemble_sub_large_offset_aarch64 : assemble_sub_aarch64;
}

const char *asm_binop_mul_template(int use_large)
{
    if (is_x86())
        return use_large ? assemble_mul_large_offset_x86_64 : assemble_mul_x86_64;
    return use_large ? assemble_mul_large_offset_aarch64 : assemble_mul_aarch64;
}

const char *asm_binop_div_template(int use_large)
{
    if (is_x86())
        return use_large ? assemble_div_large_offset_x86_64 : assemble_div_x86_64;
    return use_large ? assemble_div_large_offset_aarch64 : assemble_div_aarch64;
}

void asm_append_store_param_to_fp(char **s, int var_offset, int param_index, const char *comment)
{
    if (is_x86()) {
        static const char *regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        char buf[160];
        if (param_index < 6)
            snprintf(buf, sizeof(buf), "\n# %s\nmov [rbp+%d], %s\n",
                     comment, var_offset, regs[param_index]);
        else
            snprintf(buf, sizeof(buf), "\n# %s\nmov rax, [rbp+%d]\nmov [rbp+%d], rax\n",
                     comment, 16 + (param_index - 6) * 16, var_offset);
        asm_append(s, buf);
    } else {
        char *instr = asm_store_to_fp_instr(var_offset, param_index, comment);
        asm_append(s, instr);
        free(instr);
    }
}

void asm_append_function_frame_reserve(char **s, int bytes)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "\nsub rsp, %d\n", bytes);
    else
        snprintf(buf, sizeof(buf), "\nsub sp, sp, #%d\n", bytes);
    asm_append(s, buf);
}

void asm_emit_binop_comparison(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs,
    int left_bool, int right_bool, int result_bool)
{
    char op_asm[1024];
    const char *suffix = asm_cmp_set_suffix(op);
    if (is_x86()) {
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# comparison\n"
                "cmp eax, ebx\n"
                "set%s al\n"
                "movzx eax, al\n"
                "mov rcx, rbp\n"
                "sub rcx, %d\n"
                "mov [rcx], %s\n",
                suffix, result_abs, result_bool ? "al" : "eax");
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# comparison\n"
                "cmp eax, ebx\n"
                "set%s al\n"
                "movzx eax, al\n"
                "mov [rbp+%d], %s\n",
                suffix, result_off, result_bool ? "al" : "eax");
        }
    } else {
        const char *result_str = result_bool ? "strb" : "str";
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# comparison\n"
                "cmp w0, w1\n"
                "cset w0, %s\n"
                "sub x4, fp, #%d\n%s w0, [x4]\n",
                suffix, result_abs, result_str);
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# comparison\n"
                "cmp w0, w1\n"
                "cset w0, %s\n"
                "%s w0, [fp, #%d]\n",
                suffix, result_str, result_off);
        }
    }
    asm_append(s, op_asm);
}

void asm_emit_binop_logical(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs,
    int left_bool, int right_bool, int result_bool)
{
    char op_asm[1024];
    const char *left_ldr = left_bool ? "ldrb" : "ldr";
    const char *right_ldr = right_bool ? "ldrb" : "ldr";
    const char *result_str = result_bool ? "strb" : "str";
    const char *logic = (op == AND_TOKEN) ? "and" : (is_x86() ? "or" : "orr");
    if (is_x86()) {
        (void)left_ldr;
        (void)right_ldr;
        (void)result_str;
        (void)logic;
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# logical op\n"
                "mov rcx, rbp\nsub rcx, %d\nmovzx eax, byte ptr [rcx]\n"
                "mov rcx, rbp\nsub rcx, %d\nmovzx ebx, byte ptr [rcx]\n"
                "test eax, eax\nsetne al\n"
                "test ebx, ebx\nsetne bl\n"
                "%s eax, ebx\n"
                "mov rcx, rbp\nsub rcx, %d\nmov [rcx], al\n",
                left_abs, right_abs,
                op == AND_TOKEN ? "and" : "or",
                result_abs);
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# logical op\n"
                "movzx eax, byte ptr [rbp+%d]\n"
                "movzx ebx, byte ptr [rbp+%d]\n"
                "test eax, eax\nsetne al\n"
                "test ebx, ebx\nsetne bl\n"
                "%s eax, ebx\n"
                "mov byte ptr [rbp+%d], al\n",
                left_off, right_off,
                op == AND_TOKEN ? "and" : "or",
                result_off);
        }
    } else {
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# logical op\n"
                "sub x4, fp, #%d\n%s w0, [x4]\n"
                "sub x4, fp, #%d\n%s w1, [x4]\n"
                "cmp w0, #0\ncset w0, ne\n"
                "cmp w1, #0\ncset w1, ne\n"
                "%s w0, w0, w1\n"
                "sub x4, fp, #%d\n%s w0, [x4]\n",
                left_abs, left_ldr, right_abs, right_ldr,
                logic, result_abs, result_str);
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# logical op\n"
                "%s w0, [fp, #%d]\n%s w1, [fp, #%d]\n"
                "cmp w0, #0\ncset w0, ne\n"
                "cmp w1, #0\ncset w1, ne\n"
                "%s w0, w0, w1\n"
                "%s w0, [fp, #%d]\n",
                left_ldr, left_off, right_ldr, right_off,
                logic, result_str, result_off);
        }
    }
    asm_append(s, op_asm);
}

void asm_emit_binop_bitwise(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs)
{
    char op_asm[1024];
    const char *bitop = (op == BITAND_TOKEN) ? "and" : (is_x86() ? "or" : "orr");
    if (is_x86()) {
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# bitwise op\n"
                "mov rcx, rbp\nsub rcx, %d\nmov eax, [rcx]\n"
                "mov rcx, rbp\nsub rcx, %d\nmov ebx, [rcx]\n"
                "%s eax, ebx\n"
                "mov rcx, rbp\nsub rcx, %d\nmov [rcx], eax\n",
                left_abs, right_abs, bitop, result_abs);
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# bitwise op\n"
                "mov eax, [rbp+%d]\nmov ebx, [rbp+%d]\n"
                "%s eax, ebx\nmov [rbp+%d], eax\n",
                left_off, right_off, bitop, result_off);
        }
    } else {
        if (use_large) {
            snprintf(op_asm, sizeof(op_asm),
                "# bitwise op\n"
                "sub x4, fp, #%d\nldr w0, [x4]\n"
                "sub x4, fp, #%d\nldr w1, [x4]\n"
                "%s w0, w0, w1\n"
                "sub x4, fp, #%d\nstr w0, [x4]\n",
                left_abs, right_abs, bitop, result_abs);
        } else {
            snprintf(op_asm, sizeof(op_asm),
                "# bitwise op\n"
                "ldr w0, [fp, #%d]\nldr w1, [fp, #%d]\n"
                "%s w0, w0, w1\n"
                "str w0, [fp, #%d]\n",
                left_off, right_off, bitop, result_off);
        }
    }
    asm_append(s, op_asm);
}

void asm_append_jmp(char **s, const char *label)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "jmp %s\n", label);
    else
        snprintf(buf, sizeof(buf), "b %s\n", label);
    asm_append(s, buf);
}

void asm_append_jz_word(char **s, const char *label)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "test eax, eax\njz %s\n", label);
    else
        snprintf(buf, sizeof(buf), "cbz w0, %s\n", label);
    asm_append(s, buf);
}

void asm_append_jnz_word(char **s, const char *label)
{
    char buf[64];
    if (is_x86())
        snprintf(buf, sizeof(buf), "test eax, eax\njnz %s\n", label);
    else
        snprintf(buf, sizeof(buf), "cbnz w0, %s\n", label);
    asm_append(s, buf);
}

void asm_append_pop_expr_ptr(char **s, const char *comment)
{
    asm_append_pop_ptr_to_reg(s, 0, comment);
}

void asm_append_pop_expr_word(char **s, int is_bool, const char *comment)
{
    asm_append_pop_word_to_reg(s, 0, is_bool, comment);
}

void asm_append_push_expr_ptr(char **s)
{
    asm_append_push_ptr_from_reg(s, 0);
}

void asm_append_push_expr_word(char **s)
{
    asm_append_push_word_from_reg(s, 0);
}

void asm_append_load_binop_operand(char **s, AST_t *node, dynamic_list_t *list,
                                    int reg, const char *comment)
{
    (void)list;
    node = unwrap_comp(node);
    if (!node)
        return;
    int is_bool = node->datatype == TYPE_BOOL;
    if (asm_operand_on_stack(node)) {
        asm_append_pop_word_to_reg(s, reg, is_bool, comment);
        return;
    }
    int off = node->stack_index * -16;
    int abs_off = off < 0 ? -off : off;
    if (is_x86() && reg != 0) {
        static const char *regs32[] = {"eax", "ebx", "ecx", "edx", "r8d", "r9d"};
        const char *dest = regs32[reg < 6 ? reg : 0];
        char instr[256];
        if (is_bool) {
            if (abs_off <= 255)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmovzx %s, byte ptr [rbp-%d]\n", comment, dest, abs_off);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmovzx %s, byte ptr [rcx]\n",
                         comment, abs_off, dest);
        } else if (node->datatype == TYPE_INT || IS_ARRAY_TYPE(node->datatype)) {
            if (abs_off <= 255)
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov %s, dword ptr [rbp-%d]\n", comment, dest, abs_off);
            else
                snprintf(instr, sizeof(instr),
                         "\n# %s\nmov rcx, rbp\nsub rcx, %d\nmov %s, dword ptr [rcx]\n",
                         comment, abs_off, dest);
        } else {
            asm_append_load_from_fp(s, off, reg, comment);
            return;
        }
        asm_append(s, instr);
        return;
    }
    if (is_bool)
        asm_append_load_b_from_fp_reg(s, off, reg, comment);
    else if (node->datatype == TYPE_INT || IS_ARRAY_TYPE(node->datatype))
        asm_append_load_w_from_fp_reg(s, off, reg, comment);
    else
        asm_append_load_from_fp(s, off, reg, comment);
}

void asm_append_mov_word_reg1_from_reg0(char **s)
{
    asm_append(s, is_x86() ? "mov ebx, eax\n" : "mov w1, w0\n");
}

void asm_append_add_word_regs(char **s)
{
    asm_append(s, is_x86() ? "add eax, ebx\n" : "add w0, w0, w1\n");
}

void asm_append_sub_word_regs(char **s)
{
    asm_append(s, is_x86() ? "sub eax, ebx\n" : "sub w0, w0, w1\n");
}

void asm_append_call_runtime(char **s, const char *name, const char *comment)
{
    asm_append_call(s, name, comment);
}

void asm_append_virtual_method_call(char **s, int vtable_slot)
{
    char buf[256];
    int off = vtable_slot * 8;
    if (is_x86()) {
        snprintf(buf, sizeof(buf),
                 "\n# virtual method call slot %d\n"
                 "mov rcx, [rax]\n"
                 "call [rcx + %d]\n",
                 vtable_slot, off);
    } else {
        snprintf(buf, sizeof(buf),
                 "\n# virtual method call slot %d\n"
                 "ldr x9, [x0]\n"
                 "ldr x9, [x9, #%d]\n"
                 "blr x9\n",
                 vtable_slot, off);
    }
    asm_append(s, buf);
}

void asm_append_heap_vtable_init(char **s, int stack_index, const char *type_name)
{
    if (!type_name)
        return;
    char label[128];
    snprintf(label, sizeof(label), "%s__vtable", type_name);
    char buf[384];
    int abs_off = stack_index * 16;
    if (is_x86()) {
        if (abs_off <= 255)
            snprintf(buf, sizeof(buf),
                     "\n# init heap vtable for %s\n"
                     "mov rax, [rbp-%d]\n"
                     "lea rcx, [%s]\n"
                     "mov [rax], rcx\n",
                     type_name, abs_off, label);
        else
            snprintf(buf, sizeof(buf),
                     "\n# init heap vtable for %s\n"
                     "mov rcx, rbp\nsub rcx, %d\nmov rax, [rcx]\n"
                     "lea rcx, [%s]\n"
                     "mov [rax], rcx\n",
                     type_name, abs_off, label);
    } else {
        if (abs_off <= 255)
            snprintf(buf, sizeof(buf),
                     "\n# init heap vtable for %s\n"
                     "ldr x0, [fp, #-%d]\n"
                     "adrp x1, %s@PAGE\n"
                     "add x1, x1, %s@PAGEOFF\n"
                     "str x1, [x0]\n",
                     type_name, abs_off, label, label);
        else
            snprintf(buf, sizeof(buf),
                     "\n# init heap vtable for %s\n"
                     "sub x4, fp, #%d\nldr x0, [x4]\n"
                     "adrp x1, %s@PAGE\n"
                     "add x1, x1, %s@PAGEOFF\n"
                     "str x1, [x0]\n",
                     type_name, abs_off, label, label);
    }
    asm_append(s, buf);
}

static void patch_str(char *haystack, const char *needle, const char *replacement)
{
    size_t needle_len = strlen(needle);
    size_t repl_len = strlen(replacement);
    char *pos = haystack;
    while ((pos = strstr(pos, needle)) != NULL) {
        size_t tail = strlen(pos + needle_len);
        if (repl_len != needle_len)
            memmove(pos + repl_len, pos + needle_len, tail + 1);
        memcpy(pos, replacement, repl_len);
        pos += repl_len;
    }
}

void assembly_patch_linux_output(char *ass)
{
    if (!ass || assembly_os_get() != ASSEMBLY_OS_LINUX)
        return;
    patch_str(ass, ".section __DATA,__bss", ".section .bss");
    patch_str(ass, ".section __DATA,__data", ".section .data");
    patch_str(ass, "0x2000004", "1");
    patch_str(ass, "0x2000001", "60");
    patch_str(ass, "call _pthread_create", "call pthread_create");
    patch_str(ass, "call _pthread_attr_init", "call pthread_attr_init");
    patch_str(ass, "call _pthread_attr_setstacksize", "call pthread_attr_setstacksize");
    patch_str(ass, "call _pthread_attr_destroy", "call pthread_attr_destroy");
    patch_str(ass, "call _malloc", "call malloc");
    patch_str(ass, "call _free", "call free");
    patch_str(ass, "call _memcpy", "call memcpy");
    patch_str(ass, "call _memset", "call memset");
    patch_str(ass, "call _gettimeofday", "call gettimeofday");
    patch_str(ass, ".extern _pthread_create", ".extern pthread_create");
    patch_str(ass, ".extern _pthread_attr_init", ".extern pthread_attr_init");
    patch_str(ass, ".extern _pthread_attr_setstacksize", ".extern pthread_attr_setstacksize");
    patch_str(ass, ".extern _pthread_attr_destroy", ".extern pthread_attr_destroy");
    patch_str(ass, ".extern _malloc", ".extern malloc");
    patch_str(ass, ".extern _free", ".extern free");
    patch_str(ass, ".extern _memcpy", ".extern memcpy");
    patch_str(ass, ".extern _memset", ".extern memset");
    patch_str(ass, ".extern _gettimeofday", ".extern gettimeofday");
}

void assembly_patch_macos_x86_output(char *ass)
{
    if (!ass || assembly_os_get() != ASSEMBLY_OS_MACOS ||
        assembly_target_get() != ASSEMBLY_TARGET_X86_64)
        return;
    patch_str(ass, ".section .rodata", ".section __TEXT,__cstring,cstring_literals");
    patch_str(ass, ".section .data", ".section __DATA,__data");
    patch_str(ass, ".align 1", ".p2align 1");
    patch_str(ass, ".asciz", ".asciz");
}

static int rt_err_site_seq = 0;

void asm_append_runtime_err_site(char **s, const AST_t *ast, const char *kind,
                                 char *label_buf, size_t label_cap)
{
    snprintf(label_buf, label_cap, "_rt_site_%d", rt_err_site_seq++);
    char skip_label[32];
    snprintf(skip_label, sizeof(skip_label), "_rt_skip_%s", label_buf + 1);

    const char *file = (ast && ast->source_file) ? ast->source_file : "unknown";
    int line = (ast && ast->source_line > 0) ? ast->source_line : 0;
    char msg[480];
    snprintf(msg, sizeof(msg), "Runtime Error: %s at %s:%d\\n", kind, file, line);
    char frag[768];
    snprintf(frag, sizeof(frag),
             "b %s\n"
             "%s:\n"
             ".asciz \"%s\"\n"
             ".p2align 2\n"
             "%s:\n",
             skip_label, label_buf, msg, skip_label);
    asm_append(s, frag);
}

void asm_append_load_rt_err_ptr(char **s, const char *label, int reg_index)
{
    char frag[128];
    if (is_x86()) {
        const char *reg = (reg_index == 2) ? "rdx" : "rsi";
        snprintf(frag, sizeof(frag), "lea %s, [rip + %s]\n", reg, label);
    } else {
        const char *reg = (reg_index == 2) ? "x2" : "x1";
        snprintf(frag, sizeof(frag),
                 "adrp %s, %s@PAGE\nadd %s, %s, %s@PAGEOFF\n",
                 reg, label, reg, reg, label);
    }
    asm_append(s, frag);
}

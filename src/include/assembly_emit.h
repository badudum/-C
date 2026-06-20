#ifndef ASSEMBLY_EMIT_H
#define ASSEMBLY_EMIT_H

#include "AST.h"
#include "list.h"

void asm_append(char **s, const char *fragment);
void asm_append_return_epilogue(char **s);

void asm_append_load_from_fp(char **s, int offset, int reg, const char *comment);
void asm_append_load_w_from_fp(char **s, int offset, const char *comment);
void asm_append_store_w_to_fp(char **s, int offset, const char *comment);
void asm_append_load_b_from_fp(char **s, int offset, const char *comment);
void asm_append_store_b_to_fp(char **s, int offset, const char *comment);
void asm_append_store_to_fp(char **s, int offset, int reg, const char *comment);

void asm_append_store_value_to_fp_offset(char **s, int fp_offset, int dt, const char *comment);
void asm_append_load_value_from_fp_offset(char **s, int fp_offset, int dt, const char *comment);

char *asm_store_to_fp_instr(int offset, int reg, const char *comment);

void asm_append_load_call_arg_to_reg(char **s, AST_t *arg, int reg,
                                     dynamic_list_t *list, const char *comment);

void asm_append_pop_ptr_to_reg(char **s, int reg, const char *comment);
void asm_append_pop_word_to_reg(char **s, int reg, int is_bool, const char *comment);
void asm_append_push_ptr_from_reg(char **s, int reg);
void asm_append_push_word_from_reg(char **s, int reg);
void asm_append_discard_stack_slot(char **s);
void asm_append_store_call_result(char **s);
void asm_append_call(char **s, const char *name, const char *comment);
void asm_append_reserve_stack_args(char **s, unsigned int stack_bytes);
void asm_append_cleanup_stack_args(char **s, unsigned int stack_bytes);
void asm_append_store_stack_arg(char **s, int stack_offset);

void asm_append_dupe_fn_load(char **s, const char *name);
void asm_append_mov_word_reg(char **s, int dst, int src);
void asm_append_int_return(char **s, int value);
void asm_append_branch_if_zero(char **s, const char *label);
void asm_append_branch_if_nonzero(char **s, const char *label);
void asm_append_logical_not_word(char **s);
void asm_append_bitwise_not_word(char **s);
void asm_append_add_word_imm(char **s, int imm);
void asm_append_sub_word_imm(char **s, int imm);

const char *asm_cmp_set_suffix(int op);
const char *asm_binop_add_template(int use_large);
const char *asm_binop_sub_template(int use_large);
const char *asm_binop_mul_template(int use_large);
void asm_append_frag(char **s, const char *aarch64, const char *x86_64);
const char *asm_binop_div_template(int use_large);
void asm_append_store_param_to_fp(char **s, int var_offset, int param_index, const char *comment);
void asm_append_function_frame_reserve(char **s, int bytes);
void asm_emit_binop_comparison(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs,
    int left_bool, int right_bool, int result_bool);
void asm_emit_binop_logical(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs,
    int left_bool, int right_bool, int result_bool);
void asm_emit_binop_bitwise(char **s, int op, int use_large,
    int left_off, int right_off, int result_off,
    int left_abs, int right_abs, int result_abs);

void asm_append_jmp(char **s, const char *label);
void asm_append_jz_word(char **s, const char *label);
void asm_append_jnz_word(char **s, const char *label);
void asm_append_pop_expr_ptr(char **s, const char *comment);
void asm_append_pop_expr_word(char **s, int is_bool, const char *comment);
void asm_append_push_expr_ptr(char **s);
void asm_append_push_expr_word(char **s);
void asm_append_load_binop_operand(char **s, AST_t *node, dynamic_list_t *list,
                                    int reg, const char *comment);
void asm_append_mov_word_reg1_from_reg0(char **s);
void asm_append_add_word_regs(char **s);
void asm_append_sub_word_regs(char **s);
void asm_append_call_runtime(char **s, const char *name, const char *comment);
void assembly_patch_linux_output(char *ass);

#endif

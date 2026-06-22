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

void asm_append_load_cust_receiver_to_reg(char **s, AST_t *arg, int reg, const char *comment);
void asm_append_load_cust_field_receiver_to_reg(char **s, int base_stack_index,
                                                int field_byte_offset, int reg,
                                                const char *comment);
void asm_append_load_heap_cust_field_receiver_to_reg(char **s, int base_stack_index,
                                                     int field_byte_offset, int reg,
                                                     const char *comment);
void asm_append_heap_field_load(char **s, int base_stack_index, int byte_offset,
                                int dt, const char *comment);
void asm_append_heap_field_store(char **s, int base_stack_index, int byte_offset,
                                 int dt, const char *comment);
int asm_arg_needs_cust_receiver(AST_t *arg, dynamic_list_t *list);

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
const char *asm_binop_mod_template(int use_large);
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
int asm_resolve_arg_datatype(AST_t *arg, dynamic_list_t *list);
void asm_append_hello_convert_loaded(char **s, int dt, int abs_off);
void asm_append_virtual_method_call(char **s, int vtable_slot);
void asm_append_heap_vtable_init(char **s, int stack_index, const char *type_name);
void assembly_patch_linux_output(char *ass);
void assembly_patch_macos_x86_output(char *ass);
void assembly_patch_macos_runtime_symbols(char *ass);

/* Emit a unique runtime error string label; returns label name in label_buf. */
void asm_append_runtime_err_site(char **s, const AST_t *ast, const char *kind,
                                 char *label_buf, size_t label_cap);
void asm_append_load_rt_err_ptr(char **s, const char *label, int reg_index);

int asm_numeric_is_wide(int dt);
void asm_append_store_x_to_fp(char **s, int offset, const char *comment);
void asm_append_load_x_from_fp(char **s, int offset, const char *comment);
void asm_append_store_numeric_fp(char **s, int src_off, int dst_off, int dst_dt, int src_dt,
                                 const char *cmt);
void asm_emit_numeric_binop(char **s, AST_t *ast, dynamic_list_t *list);
void asm_emit_div_zero_check(char **s, int right_off, int right_abs, AST_t *site);
void asm_emit_compound_numeric(char **s, AST_t *assign, AST_t *rhs, int binop_op);
void asm_append_fp_load(char **s, const char *reg, int offset, const char *comment);
void asm_append_fp_store(char **s, const char *reg, int offset, const char *comment);
void asm_append_wide_int_compare(char **s, int op, int bits,
    int left_off, int right_off, int result_off);

#endif

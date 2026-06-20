#ifndef BORROW_H
#define BORROW_H

#include "AST.h"
#include "list.h"

typedef struct borrow_ctx borrow_ctx_t;

typedef struct borrow_snapshot borrow_snapshot_t;

borrow_ctx_t *borrow_ctx_new(void);
void borrow_ctx_free(borrow_ctx_t *ctx);

void borrow_func_enter(borrow_ctx_t *ctx);
void borrow_func_leave(borrow_ctx_t *ctx);

void borrow_block_enter(borrow_ctx_t *ctx);
void borrow_block_leave(borrow_ctx_t *ctx);

void borrow_set_error_at(borrow_ctx_t *ctx, const AST_t *node);

void borrow_declare_owner(borrow_ctx_t *ctx, const char *owner_key);
void borrow_mark_moved(borrow_ctx_t *ctx, const char *owner_key);
void borrow_revive(borrow_ctx_t *ctx, const char *owner_key);

void borrow_check_use(borrow_ctx_t *ctx, const char *owner_key, const char *context);
void borrow_check_can_move(borrow_ctx_t *ctx, const char *owner_key, const char *context);
void borrow_check_owner_frozen(borrow_ctx_t *ctx, const char *owner_key, const char *context);
void borrow_check_owner_writable(borrow_ctx_t *ctx, const char *owner_key, const char *context);

void borrow_check_no_active_loans(borrow_ctx_t *ctx, const char *owner_key, const char *context);

int borrow_is_moved(borrow_ctx_t *ctx, const char *owner_key);

void borrow_create_loan(borrow_ctx_t *ctx, const char *loan_var, const char *owner_key, int is_mut);
void borrow_release_loan_var(borrow_ctx_t *ctx, const char *loan_var);

char *borrow_owner_key_var(const char *var_name);
char *borrow_owner_key_field(const char *var_name, const char *field_name);
char *borrow_owner_key_from_expr(AST_t *expr, dynamic_list_t *list);

void borrow_check_adr_expr(borrow_ctx_t *ctx, AST_t *expr, dynamic_list_t *list, const char *context);
void borrow_check_can_move_expr(borrow_ctx_t *ctx, AST_t *expr, dynamic_list_t *list, const char *context);

void borrow_handle_owner_reassignment(borrow_ctx_t *ctx, AST_t *variable, dynamic_list_t *list);
void borrow_handle_loan_assignment(borrow_ctx_t *ctx, AST_t *variable, dynamic_list_t *list);
void borrow_register_cust_adr_fields(borrow_ctx_t *ctx, const char *var_name, int cust_id);

borrow_snapshot_t *borrow_snapshot_take(borrow_ctx_t *ctx);
void borrow_snapshot_restore(borrow_ctx_t *ctx, borrow_snapshot_t *snap);
void borrow_snapshot_merge(borrow_ctx_t *ctx, borrow_snapshot_t *then_snap,
                           borrow_snapshot_t *else_snap);
void borrow_snapshot_free(borrow_snapshot_t *snap);

const char *borrow_loan_owner(borrow_ctx_t *ctx, const char *loan_var);

#endif

#include "../include/borrow.h"
#include "../include/cust.h"
#include "../include/types.h"
#include "../include/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;
    int moved;
    int shared_loans;
    int mut_loan;
} adr_owner_t;

typedef struct {
    char *loan_var;
    char *owner_key;
    int is_mut;
} loan_entry_t;

struct borrow_snapshot {
    dynamic_list_t *owners;
};

struct borrow_ctx {
    dynamic_list_t *owners;
    dynamic_list_t *loan_frame_stack;
    int in_function;
    const AST_t *error_at;
};

static adr_owner_t *owner_find(borrow_ctx_t *ctx, const char *key)
{
    if (!ctx || !ctx->owners || !key)
        return 0;
    for (int i = (int)ctx->owners->size - 1; i >= 0; i--) {
        adr_owner_t *st = (adr_owner_t *)ctx->owners->items[i];
        if (st->key && strcmp(st->key, key) == 0)
            return st;
    }
    return 0;
}

static adr_owner_t *owner_get_or_create(borrow_ctx_t *ctx, const char *key)
{
    adr_owner_t *st = owner_find(ctx, key);
    if (st)
        return st;
    st = calloc(1, sizeof(adr_owner_t));
    st->key = strdup(key);
    list_enqueue(ctx->owners, st);
    return st;
}

static dynamic_list_t *current_loan_frame(borrow_ctx_t *ctx)
{
    if (!ctx->loan_frame_stack || ctx->loan_frame_stack->size == 0)
        return 0;
    return (dynamic_list_t *)ctx->loan_frame_stack->items[ctx->loan_frame_stack->size - 1];
}

static loan_entry_t *loan_find_in_frame(dynamic_list_t *frame, const char *loan_var)
{
    if (!frame || !loan_var)
        return 0;
    for (unsigned int i = 0; i < frame->size; i++) {
        loan_entry_t *ln = (loan_entry_t *)frame->items[i];
        if (ln->loan_var && strcmp(ln->loan_var, loan_var) == 0)
            return ln;
    }
    return 0;
}

static loan_entry_t *loan_find(borrow_ctx_t *ctx, const char *loan_var)
{
    if (!ctx || !loan_var)
        return 0;
    for (int i = (int)ctx->loan_frame_stack->size - 1; i >= 0; i--) {
        dynamic_list_t *frame = (dynamic_list_t *)ctx->loan_frame_stack->items[i];
        loan_entry_t *ln = loan_find_in_frame(frame, loan_var);
        if (ln)
            return ln;
    }
    return 0;
}

static void owner_error(borrow_ctx_t *ctx, const char *msg, const char *key, const char *context)
{
    if (context && context[0])
        compile_error_ast(ctx ? ctx->error_at : NULL, "Borrow error: %s '%s'%s", msg, key, context);
    else
        compile_error_ast(ctx ? ctx->error_at : NULL, "Borrow error: %s '%s'", msg, key);
}

void borrow_set_error_at(borrow_ctx_t *ctx, const AST_t *node)
{
    if (ctx)
        ctx->error_at = node;
}

static int is_adr_owner_expr(AST_t *expr, dynamic_list_t *list)
{
    if (!expr)
        return 0;
    if (expr->type == VAR_AST && expr->name) {
        if (expr->datatype == TYPE_ADR)
            return 1;
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, expr->name) == 0)
                return def->datatype == TYPE_ADR;
        }
    }
    if (expr->type == FIELD_ACCESS_AST && expr->datatype == TYPE_ADR)
        return 1;
    return 0;
}

static AST_t *field_access_root(AST_t *node)
{
    while (node && node->type == FIELD_ACCESS_AST && node->left)
        node = node->left;
    return node;
}

static void build_field_path(AST_t *expr, char *buf, size_t cap)
{
    if (!expr || !buf || cap == 0 || expr->type != FIELD_ACCESS_AST)
        return;
    if (expr->left && expr->left->type == VAR_AST && expr->left->name)
        snprintf(buf, cap, "%s.%s", expr->left->name, expr->name);
    else if (expr->left && expr->left->type == FIELD_ACCESS_AST) {
        build_field_path(expr->left, buf, cap);
        size_t len = strlen(buf);
        if (len + strlen(expr->name) + 2 < cap)
            snprintf(buf + len, cap - len, ".%s", expr->name);
    }
}

static AST_t *assignment_rhs(AST_t *parent)
{
    if (!parent)
        return 0;
    if (parent->type == COMP_AST && parent->children && parent->children->size == 1)
        return (AST_t *)parent->children->items[0];
    return parent;
}

static int is_fresh_adr_rhs(AST_t *rhs)
{
    rhs = assignment_rhs(rhs);
    if (!rhs || rhs->type != CALL_AST || !rhs->name)
        return 0;
    return strcmp(rhs->name, "rent") == 0 || strcmp(rhs->name, "rentMul") == 0;
}

static int is_rentgrow_rhs(AST_t *rhs)
{
    rhs = assignment_rhs(rhs);
    return rhs && rhs->type == CALL_AST && rhs->name && strcmp(rhs->name, "RentGrow") == 0;
}

borrow_ctx_t *borrow_ctx_new(void)
{
    borrow_ctx_t *ctx = calloc(1, sizeof(borrow_ctx_t));
    ctx->owners = init_list(sizeof(adr_owner_t *));
    ctx->loan_frame_stack = init_list(sizeof(dynamic_list_t *));
    return ctx;
}

void borrow_ctx_free(borrow_ctx_t *ctx)
{
    if (!ctx)
        return;
    if (ctx->owners) {
        for (unsigned int i = 0; i < ctx->owners->size; i++) {
            adr_owner_t *st = (adr_owner_t *)ctx->owners->items[i];
            free(st->key);
            free(st);
        }
        free(ctx->owners->items);
        free(ctx->owners);
    }
    if (ctx->loan_frame_stack) {
        for (unsigned int i = 0; i < ctx->loan_frame_stack->size; i++) {
            dynamic_list_t *frame = (dynamic_list_t *)ctx->loan_frame_stack->items[i];
            for (unsigned int j = 0; j < frame->size; j++) {
                loan_entry_t *ln = (loan_entry_t *)frame->items[j];
                free(ln->loan_var);
                free(ln->owner_key);
                free(ln);
            }
            free(frame->items);
            free(frame);
        }
        free(ctx->loan_frame_stack->items);
        free(ctx->loan_frame_stack);
    }
    free(ctx);
}

void borrow_func_enter(borrow_ctx_t *ctx)
{
    if (!ctx)
        return;
    ctx->in_function = 1;
    dynamic_list_t *frame = init_list(sizeof(loan_entry_t *));
    list_enqueue(ctx->loan_frame_stack, frame);
}

void borrow_func_leave(borrow_ctx_t *ctx)
{
    if (!ctx)
        return;
    borrow_block_leave(ctx);
    ctx->in_function = 0;
    for (unsigned int i = 0; i < ctx->owners->size; i++) {
        adr_owner_t *st = (adr_owner_t *)ctx->owners->items[i];
        free(st->key);
        free(st);
    }
    ctx->owners->size = 0;
}

void borrow_block_enter(borrow_ctx_t *ctx)
{
    if (!ctx || !ctx->in_function)
        return;
    dynamic_list_t *frame = init_list(sizeof(loan_entry_t *));
    list_enqueue(ctx->loan_frame_stack, frame);
}

void borrow_block_leave(borrow_ctx_t *ctx)
{
    if (!ctx || !ctx->loan_frame_stack || ctx->loan_frame_stack->size == 0)
        return;
    dynamic_list_t *frame = (dynamic_list_t *)ctx->loan_frame_stack->items[ctx->loan_frame_stack->size - 1];
    for (unsigned int i = 0; i < frame->size; i++) {
        loan_entry_t *ln = (loan_entry_t *)frame->items[i];
        adr_owner_t *owner = owner_find(ctx, ln->owner_key);
        if (owner) {
            if (ln->is_mut)
                owner->mut_loan = 0;
            else if (owner->shared_loans > 0)
                owner->shared_loans--;
        }
        free(ln->loan_var);
        free(ln->owner_key);
        free(ln);
    }
    free(frame->items);
    free(frame);
    ctx->loan_frame_stack->size--;
}

void borrow_declare_owner(borrow_ctx_t *ctx, const char *owner_key)
{
    if (!ctx || !owner_key)
        return;
    owner_get_or_create(ctx, owner_key);
}

void borrow_mark_moved(borrow_ctx_t *ctx, const char *owner_key)
{
    adr_owner_t *st = owner_get_or_create(ctx, owner_key);
    if (st)
        st->moved = 1;
}

void borrow_revive(borrow_ctx_t *ctx, const char *owner_key)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (st)
        st->moved = 0;
}

void borrow_check_use(borrow_ctx_t *ctx, const char *owner_key, const char *context)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (st && st->moved)
        owner_error(ctx, "use of moved adr", owner_key, context);
}

void borrow_check_can_move(borrow_ctx_t *ctx, const char *owner_key, const char *context)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (!st)
        return;
    if (st->moved)
        owner_error(ctx, "use of moved adr", owner_key, context);
    if (st->mut_loan)
        owner_error(ctx, "cannot move adr while mutably borrowed", owner_key, context);
    if (st->shared_loans > 0)
        owner_error(ctx, "cannot move adr while shared borrow is active", owner_key, context);
}

void borrow_check_owner_frozen(borrow_ctx_t *ctx, const char *owner_key, const char *context)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (st && st->mut_loan)
        owner_error(ctx, "owner frozen while mutably borrowed", owner_key, context);
}

void borrow_check_owner_writable(borrow_ctx_t *ctx, const char *owner_key, const char *context)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (!st)
        return;
    if (st->mut_loan)
        owner_error(ctx, "owner frozen while mutably borrowed", owner_key, context);
    if (st->shared_loans > 0)
        owner_error(ctx, "owner frozen while shared borrow is active", owner_key, context);
}

void borrow_check_no_active_loans(borrow_ctx_t *ctx, const char *owner_key, const char *context)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    if (!st)
        return;
    if (st->mut_loan)
        owner_error(ctx, "cannot use adr while mutably borrowed", owner_key, context);
    if (st->shared_loans > 0)
        owner_error(ctx, "cannot use adr while shared borrow is active", owner_key, context);
}

int borrow_is_moved(borrow_ctx_t *ctx, const char *owner_key)
{
    adr_owner_t *st = owner_find(ctx, owner_key);
    return st && st->moved;
}

void borrow_create_loan(borrow_ctx_t *ctx, const char *loan_var, const char *owner_key, int is_mut)
{
    dynamic_list_t *frame = current_loan_frame(ctx);
    if (!frame || !loan_var || !owner_key)
        return;

    adr_owner_t *owner = owner_get_or_create(ctx, owner_key);
    borrow_check_use(ctx, owner_key, " (borrow of moved adr)");
    if (is_mut) {
        if (owner->mut_loan || owner->shared_loans > 0)
            owner_error(ctx, "cannot create mutable borrow while other borrows exist", owner_key, "");
        owner->mut_loan = 1;
    } else {
        if (owner->mut_loan)
            owner_error(ctx, "cannot create shared borrow while mutably borrowed", owner_key, "");
        owner->shared_loans++;
    }

    loan_entry_t *ln = calloc(1, sizeof(loan_entry_t));
    ln->loan_var = strdup(loan_var);
    ln->owner_key = strdup(owner_key);
    ln->is_mut = is_mut ? 1 : 0;
    list_enqueue(frame, ln);
}

void borrow_release_loan_var(borrow_ctx_t *ctx, const char *loan_var)
{
    loan_entry_t *ln = loan_find(ctx, loan_var);
    if (!ln)
        return;
    adr_owner_t *owner = owner_find(ctx, ln->owner_key);
    if (owner) {
        if (ln->is_mut)
            owner->mut_loan = 0;
        else if (owner->shared_loans > 0)
            owner->shared_loans--;
    }
}

char *borrow_owner_key_var(const char *var_name)
{
    return var_name ? strdup(var_name) : 0;
}

char *borrow_owner_key_field(const char *var_name, const char *field_name)
{
    if (!var_name || !field_name)
        return 0;
    char *key = calloc(strlen(var_name) + strlen(field_name) + 2, 1);
    snprintf(key, strlen(var_name) + strlen(field_name) + 2, "%s.%s", var_name, field_name);
    return key;
}

static int field_access_is_adr(AST_t *expr, dynamic_list_t *list)
{
    if (!expr || expr->type != FIELD_ACCESS_AST || !expr->name)
        return 0;
    if (expr->datatype == TYPE_ADR)
        return 1;

    AST_t *container = expr->left;
    if (!container)
        return 0;

    if (container->type == VAR_AST && container->name) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, container->name) == 0) {
                int cid = -1;
                if (IS_CUST_TYPE(def->datatype))
                    cid = CUST_TYPE_ID(def->datatype);
                else if (IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                    cid = def->int_value;
                if (cid >= 0) {
                    cust_field_t *field = cust_field_lookup(cid, expr->name, NULL);
                    return field && field->datatype == TYPE_ADR;
                }
            }
        }
    }
    if (container->type == FIELD_ACCESS_AST &&
        IS_CUST_TYPE(container->datatype)) {
        cust_field_t *field = cust_field_lookup(CUST_TYPE_ID(container->datatype), expr->name, NULL);
        return field && field->datatype == TYPE_ADR;
    }
    return 0;
}

char *borrow_owner_key_from_expr(AST_t *expr, dynamic_list_t *list)
{
    if (!expr)
        return 0;
    if (expr->type == VAR_AST && expr->name && is_adr_owner_expr(expr, list))
        return borrow_owner_key_var(expr->name);
    if (expr->type == FIELD_ACCESS_AST && field_access_is_adr(expr, list)) {
        char buf[256];
        build_field_path(expr, buf, sizeof(buf));
        if (buf[0])
            return strdup(buf);
    }
    return 0;
}

void borrow_check_adr_expr(borrow_ctx_t *ctx, AST_t *expr, dynamic_list_t *list, const char *context)
{
    char *key = borrow_owner_key_from_expr(expr, list);
    if (key) {
        borrow_check_use(ctx, key, context);
        free(key);
        return;
    }
    if (expr && expr->type == VAR_AST && expr->name &&
        (expr->datatype == TYPE_ADR_SHREF || expr->datatype == TYPE_ADR_MUTREF)) {
        const char *owner = borrow_loan_owner(ctx, expr->name);
        if (!owner) {
            compile_error_ast(ctx ? ctx->error_at : expr, "Borrow error: use of expired loan '%s'", expr->name);
        }
        borrow_check_use(ctx, owner, context);
    }
}

void borrow_check_can_move_expr(borrow_ctx_t *ctx, AST_t *expr, dynamic_list_t *list, const char *context)
{
    char *key = borrow_owner_key_from_expr(expr, list);
    if (key) {
        borrow_check_can_move(ctx, key, context);
        free(key);
    }
}

void borrow_handle_owner_reassignment(borrow_ctx_t *ctx, AST_t *variable, dynamic_list_t *list)
{
    if (!variable || !variable->name || variable->datatype != TYPE_ADR || !variable->parent)
        return;

    AST_t *rhs = assignment_rhs(variable->parent);
    if (!rhs)
        return;

    borrow_declare_owner(ctx, variable->name);
    borrow_check_no_active_loans(ctx, variable->name, " (reassign while borrowed)");

    if (rhs->type == VAR_AST && is_adr_owner_expr(rhs, list)) {
        borrow_check_can_move(ctx, rhs->name, " (moved on assign)");
        borrow_mark_moved(ctx, rhs->name);
        borrow_revive(ctx, variable->name);
    } else if (is_fresh_adr_rhs(rhs)) {
        borrow_revive(ctx, variable->name);
    } else if (is_rentgrow_rhs(rhs)) {
        borrow_revive(ctx, variable->name);
        if (rhs->parent && rhs->parent->children && rhs->parent->children->size > 0) {
            AST_t *grow_src = (AST_t *)rhs->parent->children->items[0];
            char *key = borrow_owner_key_from_expr(grow_src, list);
            if (key) {
                borrow_check_can_move(ctx, key, " (RentGrow source)");
                borrow_mark_moved(ctx, key);
                free(key);
            }
        }
    }
}

void borrow_handle_loan_assignment(borrow_ctx_t *ctx, AST_t *variable, dynamic_list_t *list)
{
    if (!variable || !variable->parent)
        return;
    AST_t *rhs = assignment_rhs(variable->parent);
    if (!rhs)
        return;

    char *owner_key = borrow_owner_key_from_expr(rhs, list);
    if (!owner_key) {
        compile_error_ast(ctx ? ctx->error_at : variable, "Borrow error: loan requires an adr owner expression");
    }

    int is_mut = variable->datatype == TYPE_ADR_MUTREF;
    borrow_create_loan(ctx, variable->name, owner_key, is_mut);
    free(owner_key);
}

void borrow_register_cust_adr_fields(borrow_ctx_t *ctx, const char *var_name, int cust_id)
{
    cust_type_t *type = cust_get(cust_id);
    if (!type || !type->fields)
        return;
    for (unsigned int i = 0; i < type->fields->size; i++) {
        cust_field_t *field = (cust_field_t *)type->fields->items[i];
        if (field->datatype == TYPE_ADR) {
            char *key = borrow_owner_key_field(var_name, field->name);
            borrow_declare_owner(ctx, key);
            free(key);
        }
    }
}

static adr_owner_t *snapshot_find(dynamic_list_t *snap, const char *key)
{
    if (!snap)
        return 0;
    for (unsigned int i = 0; i < snap->size; i++) {
        adr_owner_t *st = (adr_owner_t *)snap->items[i];
        if (st->key && strcmp(st->key, key) == 0)
            return st;
    }
    return 0;
}

static void snapshot_copy_owners(dynamic_list_t *dst, dynamic_list_t *src)
{
    for (unsigned int i = 0; i < src->size; i++) {
        adr_owner_t *orig = (adr_owner_t *)src->items[i];
        adr_owner_t *copy = calloc(1, sizeof(adr_owner_t));
        copy->key = strdup(orig->key);
        copy->moved = orig->moved;
        copy->shared_loans = orig->shared_loans;
        copy->mut_loan = orig->mut_loan;
        list_enqueue(dst, copy);
    }
}

borrow_snapshot_t *borrow_snapshot_take(borrow_ctx_t *ctx)
{
    borrow_snapshot_t *snap = calloc(1, sizeof(borrow_snapshot_t));
    snap->owners = init_list(sizeof(adr_owner_t *));
    snapshot_copy_owners(snap->owners, ctx->owners);
    return snap;
}

void borrow_snapshot_restore(borrow_ctx_t *ctx, borrow_snapshot_t *snap)
{
    if (!ctx || !snap)
        return;
    for (unsigned int i = 0; i < ctx->owners->size; i++) {
        adr_owner_t *st = (adr_owner_t *)ctx->owners->items[i];
        free(st->key);
        free(st);
    }
    ctx->owners->size = 0;
    snapshot_copy_owners(ctx->owners, snap->owners);
}

static void merge_owner_state(adr_owner_t *dst, adr_owner_t *a, adr_owner_t *b)
{
    dst->moved = (a && a->moved) || (b && b->moved);
    int sa = a ? a->shared_loans : 0;
    int sb = b ? b->shared_loans : 0;
    dst->shared_loans = sa > sb ? sa : sb;
    dst->mut_loan = (a && a->mut_loan) || (b && b->mut_loan);
}

void borrow_snapshot_merge(borrow_ctx_t *ctx, borrow_snapshot_t *then_snap,
                           borrow_snapshot_t *else_snap)
{
    dynamic_list_t *merged = init_list(sizeof(adr_owner_t *));
    for (unsigned int i = 0; i < ctx->owners->size; i++) {
        adr_owner_t *cur = (adr_owner_t *)ctx->owners->items[i];
        adr_owner_t *then_st = snapshot_find(then_snap ? then_snap->owners : 0, cur->key);
        adr_owner_t *else_st = snapshot_find(else_snap ? else_snap->owners : 0, cur->key);
        adr_owner_t *out = calloc(1, sizeof(adr_owner_t));
        out->key = strdup(cur->key);
        merge_owner_state(out, then_st ? then_st : cur, else_st ? else_st : cur);
        list_enqueue(merged, out);
    }
    for (unsigned int i = 0; i < ctx->owners->size; i++) {
        adr_owner_t *st = (adr_owner_t *)ctx->owners->items[i];
        free(st->key);
        free(st);
    }
    free(ctx->owners->items);
    ctx->owners = merged;
}

void borrow_snapshot_free(borrow_snapshot_t *snap)
{
    if (!snap)
        return;
    if (snap->owners) {
        for (unsigned int i = 0; i < snap->owners->size; i++) {
            adr_owner_t *st = (adr_owner_t *)snap->owners->items[i];
            free(st->key);
            free(st);
        }
        free(snap->owners->items);
        free(snap->owners);
    }
    free(snap);
}

const char *borrow_loan_owner(borrow_ctx_t *ctx, const char *loan_var)
{
    loan_entry_t *ln = loan_find(ctx, loan_var);
    return ln ? ln->owner_key : 0;
}

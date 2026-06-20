#include "../include/heap_scope.h"
#include "../include/borrow.h"
#include "../include/cust.h"
#include "../include/types.h"
#include "../include/utils.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    int cust_id;
    int stack_index;
    stackframe_t *stackframe;
    int dropped;
} heap_scope_binding_t;

void heap_scope_init(visitor_t *visitor)
{
    if (!visitor)
        return;
    visitor->heap_scope_stack = init_list(sizeof(heap_scope_binding_t *));
}

void heap_scope_free(visitor_t *visitor)
{
    if (!visitor || !visitor->heap_scope_stack)
        return;
    for (unsigned int i = 0; i < visitor->heap_scope_stack->size; i++) {
        dynamic_list_t *scope = (dynamic_list_t *)visitor->heap_scope_stack->items[i];
        for (unsigned int j = 0; j < scope->size; j++) {
            heap_scope_binding_t *b = (heap_scope_binding_t *)scope->items[j];
            free(b->name);
            free(b);
        }
        free(scope->items);
        free(scope);
    }
    free(visitor->heap_scope_stack->items);
    free(visitor->heap_scope_stack);
    visitor->heap_scope_stack = 0;
}

void heap_scope_push(visitor_t *visitor)
{
    if (!visitor)
        return;
    if (!visitor->heap_scope_stack)
        heap_scope_init(visitor);
    list_enqueue(visitor->heap_scope_stack, init_list(sizeof(heap_scope_binding_t *)));
}

static dynamic_list_t *heap_scope_current(visitor_t *visitor)
{
    if (!visitor || !visitor->heap_scope_stack || visitor->heap_scope_stack->size == 0)
        return 0;
    return (dynamic_list_t *)visitor->heap_scope_stack->items[visitor->heap_scope_stack->size - 1];
}

void heap_scope_register(visitor_t *visitor, const char *name, int cust_id,
                         int stack_index, stackframe_t *stackframe)
{
    dynamic_list_t *scope = heap_scope_current(visitor);
    if (!scope || !name || cust_id < 0)
        return;
    heap_scope_binding_t *b = calloc(1, sizeof(heap_scope_binding_t));
    b->name = strdup(name);
    b->cust_id = cust_id;
    b->stack_index = stack_index;
    b->stackframe = stackframe;
    list_enqueue(scope, b);
}

void heap_scope_mark_dropped(visitor_t *visitor, const char *name)
{
    if (!visitor || !name || !visitor->heap_scope_stack)
        return;
    for (int si = (int)visitor->heap_scope_stack->size - 1; si >= 0; si--) {
        dynamic_list_t *scope = (dynamic_list_t *)visitor->heap_scope_stack->items[si];
        for (unsigned int i = 0; i < scope->size; i++) {
            heap_scope_binding_t *b = (heap_scope_binding_t *)scope->items[i];
            if (b->name && strcmp(b->name, name) == 0) {
                b->dropped = 1;
                return;
            }
        }
    }
}

static AST_t *heap_scope_var_ref(heap_scope_binding_t *b)
{
    AST_t *recv = init_ast(VAR_AST);
    recv->name = b->name;
    recv->datatype = TYPE_ADR;
    recv->int_value = b->cust_id;
    recv->stack_index = b->stack_index;
    recv->stackframe = b->stackframe;
    return recv;
}

static AST_t *heap_scope_emit_drop(visitor_t *visitor, heap_scope_binding_t *b,
                                   dynamic_list_t *list, stackframe_t *stackframe)
{
    AST_t *recv = heap_scope_var_ref(b);
    AST_t *args = init_ast(COMP_AST);
    AST_t *call = init_ast(CALL_AST);
    call->name = mkstr("drop");
    call->left = recv;
    call->parent = args;
    return visit_caller(visitor, call, list, stackframe);
}

static AST_t *heap_scope_emit_moveout(visitor_t *visitor, heap_scope_binding_t *b,
                                      dynamic_list_t *list, stackframe_t *stackframe)
{
    AST_t *recv = heap_scope_var_ref(b);
    AST_t *args = init_ast(COMP_AST);
    list_enqueue(args->children, recv);
    AST_t *call = init_ast(CALL_AST);
    call->name = mkstr("moveOut");
    call->parent = args;
    return visit_caller(visitor, call, list, stackframe);
}

void heap_scope_pop_emit(visitor_t *visitor, AST_t *compound, dynamic_list_t *list,
                         stackframe_t *stackframe)
{
    if (!visitor || !visitor->heap_scope_stack || visitor->heap_scope_stack->size == 0)
        return;
    dynamic_list_t *scope = (dynamic_list_t *)visitor->heap_scope_stack->items[
        visitor->heap_scope_stack->size - 1];
    visitor->heap_scope_stack->size--;

    if (!compound)
        goto cleanup_scope;

    for (int i = (int)scope->size - 1; i >= 0; i--) {
        heap_scope_binding_t *b = (heap_scope_binding_t *)scope->items[i];
        if (borrow_is_moved(visitor->bctx, b->name))
            continue;
        if (!b->dropped && cust_method_by_name(b->cust_id, "drop")) {
            AST_t *drop_call = heap_scope_emit_drop(visitor, b, list, stackframe);
            list_enqueue(compound->children, drop_call);
        }
        if (!borrow_is_moved(visitor->bctx, b->name)) {
            AST_t *out_call = heap_scope_emit_moveout(visitor, b, list, stackframe);
            list_enqueue(compound->children, out_call);
        }
    }

cleanup_scope:
    for (unsigned int i = 0; i < scope->size; i++) {
        heap_scope_binding_t *b = (heap_scope_binding_t *)scope->items[i];
        free(b->name);
        free(b);
    }
    free(scope->items);
    free(scope);
}

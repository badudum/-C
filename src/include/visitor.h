#ifndef VISITOR_H
#define VISITOR_H

#include "AST.h"
#include "list.h"
#include "stackframe.h"
#include "errors.h"
#include "utils.h"
#include "borrow.h"


typedef struct VISITOR_S
{
    AST_t* object;
    borrow_ctx_t *bctx;
    int cust_access_type; /* cust type id for visibility, or -1 */
    AST_t *current_func;  /* func AST while visiting its body (for self params) */
    dynamic_list_t *heap_scope_stack; /* stack of per-scope heap binding lists */
}visitor_t;

AST_t* variable_lookup(dynamic_list_t* list, char* name);

visitor_t* init_visitor();

AST_t* visitor_visit(visitor_t* visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_compound(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_func(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_caller(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_int(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_str(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_binop(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_return(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_access(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_slice(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_array_literal(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_bool(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_if(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_unary(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_loop_until(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_for_clause(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_inc_dec(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_type_size(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_cust_def(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe);

AST_t* visit_cust_init(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe);

AST_t* visit_field_access(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe);

#endif
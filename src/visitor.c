#include "include/visitor.h"
#include "include/borrow.h"
#include "include/heap_scope.h"
#include "include/token.h"
#include "include/cust.h"
#include "include/types.h"
#include "include/interface.h"
#include "include/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERFACE_PARAM_MARK 2000

static AST_t* visit_dupe(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);
static int is_adr_var(AST_t *node, dynamic_list_t *list);
static int tag_heap_cust_from_rent(AST_t *variable, AST_t *rhs);

static int stack_lookup_for_name(stackframe_t *stackframe, const char *name,
                                 stackframe_t **out_frame)
{
    if (!stackframe || !name)
        return 0;
    if (stackframe->parent) {
        int start = stackframe->scope_base;
        for (int i = (int)stackframe->stack->size - 1; i >= start; i--) {
            char *var_name = (char*)stackframe->stack->items[i];
            if (var_name && strcmp(var_name, name) == 0) {
                if (out_frame)
                    *out_frame = stackframe;
                return i + 1;
            }
        }
        return stack_lookup_for_name(stackframe->parent, name, out_frame);
    }
    for (int i = (int)stackframe->stack->size - 1; i >= 0; i--) {
        char *var_name = (char*)stackframe->stack->items[i];
        if (var_name && strcmp(var_name, name) == 0) {
            if (out_frame)
                *out_frame = stackframe;
            return i + 1;
        }
    }
    return 0;
}

static int stack_index_for_name(stackframe_t *stackframe, const char *name)
{
    return stack_lookup_for_name(stackframe, name, 0);
}

static void borrow_handle_field_adr_assign(visitor_t *visitor, AST_t *fa, AST_t *rhs,
                                           dynamic_list_t *list)
{
    char *key = borrow_owner_key_from_expr(fa, list);
    if (!key)
        return;
    borrow_declare_owner(visitor->bctx, key);
    borrow_check_no_active_loans(visitor->bctx, key, " (field reassign while borrowed)");
    char *src_key = borrow_owner_key_from_expr(rhs, list);
    if (src_key) {
        borrow_check_can_move(visitor->bctx, src_key, " (field moved on assign)");
        borrow_mark_moved(visitor->bctx, src_key);
        borrow_revive(visitor->bctx, key);
        free(src_key);
    } else if (rhs && rhs->type == CALL_AST && rhs->name &&
               (strcmp(rhs->name, "rent") == 0 || strcmp(rhs->name, "rentMul") == 0)) {
        borrow_revive(visitor->bctx, key);
    }
    free(key);
}

static int datatype_of_var_name(dynamic_list_t *list, const char *name)
{
    if (!list || !name)
        return TYPE_UNKNOWN;
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t *def = (AST_t *)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name && strcmp(def->name, name) == 0)
            return def->datatype;
    }
    return TYPE_UNKNOWN;
}

static AST_t *field_access_root(AST_t *node)
{
    while (node && node->type == FIELD_ACCESS_AST && node->left)
        node = node->left;
    return node;
}

static int is_adr_var(AST_t *node, dynamic_list_t *list)
{
    if (!node || node->type != VAR_AST || !node->name)
        return 0;
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t *def = (AST_t*)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name &&
            strcmp(def->name, node->name) == 0)
            return IS_ADR_OWNER(def->datatype);
    }
    return 0;
}

 AST_t* variable_lookup(dynamic_list_t* list, char* name)
{
    if (!list || !name) return 0;
    for (int i = 0; i < list->size; i++)
    {
        AST_t* children = (AST_t*)list->items[i];
        if (!children) continue;
        int has_name = (children->name && strcmp(children->name, name) == 0);
        if (!has_name) continue;
        if (children->type == VAR_AST || children->type == FUNC_AST)
            return children;
        if (children->type == ASSIGNEMENT_AST)
            return children; /* variable declaration/assignment has name and stack_index */
    }
    return 0;
}

visitor_t* init_visitor()
{
    visitor_t* visitor = calloc(1, sizeof(visitor_t));
    visitor->object = init_ast(COMP_AST);
    visitor->bctx = borrow_ctx_new();
    visitor->cust_access_type = -1;
    visitor->current_func = NULL;
    heap_scope_init(visitor);
    return visitor;
}

/* Record ASSIGNEMENT_AST nodes on list so VAR_AST can resolve datatype (e.g. bool vs int loads). */
static void collect_assignments_into_list(AST_t* n, dynamic_list_t* list)
{
    if (!n) return;
    if (n->type == ASSIGNEMENT_AST) {
        list_enqueue(list, n);
        return;
    }
    if (n->type == FUNC_AST) {
        for (unsigned int i = 0; i < n->children->size; i++)
            collect_assignments_into_list((AST_t*)n->children->items[i], list);
        if (n->parent)
            collect_assignments_into_list(n->parent, list);
        return;
    }
    if (n->type == COMP_AST && n->children) {
        for (unsigned int i = 0; i < n->children->size; i++)
            collect_assignments_into_list((AST_t*)n->children->items[i], list);
    }
}

// visit the AST node, then determine what to do with it
AST_t* visitor_visit(visitor_t* visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    borrow_set_error_at(visitor->bctx, node);
    // printf("Visiting node: %d\n", node->type);
    switch (node->type)
    {
        case COMP_AST: return visit_compound(visitor, node, list, stackframe); break;
        case ASSIGNEMENT_AST: return visit_assignment(visitor, node, list, stackframe);break;
        case VAR_AST: return visit_var(visitor, node, list, stackframe);break;
        case FUNC_AST: return visit_func(visitor, node, list, stackframe);break;
        case CALL_AST: return visit_caller(visitor, node, list, stackframe);break;
        case INT_AST: return visit_int(visitor, node, list, stackframe);break;
        case STRING_AST: return visit_str(visitor, node, list, stackframe);break;
        case BINOP_AST: return visit_binop(visitor, node, list, stackframe);break;
        case RETURN_AST: return visit_return(visitor, node, list, stackframe);break;
        case ACCESS_AST: return visit_access(visitor, node, list, stackframe);break;
        case SLICE_AST: return visit_slice(visitor, node, list, stackframe);break;
        case ARRAY_LITERAL_AST: return visit_array_literal(visitor, node, list, stackframe);break;
        case BOOL_AST: return visit_bool(visitor, node, list, stackframe);break;
        case IF_AST: return visit_if(visitor, node, list, stackframe);break;
        case UNARY_AST: return visit_unary(visitor, node, list, stackframe);break;
        case LOOP_UNTIL_AST: return visit_loop_until(visitor, node, list, stackframe);break;
        case FOR_CLAUSE_AST: return visit_for_clause(visitor, node, list, stackframe);break;
        case INC_DEC_AST: return visit_inc_dec(visitor, node, list, stackframe);break;
        case DUPE_AST: return visit_dupe(visitor, node, list, stackframe);break;
        case TYPE_SIZE_AST: return visit_type_size(visitor, node, list, stackframe);break;
        case CUST_DEF_AST: return visit_cust_def(visitor, node, list, stackframe);break;
        case CUST_INIT_AST: return visit_cust_init(visitor, node, list, stackframe);break;
        case FIELD_ACCESS_AST: return visit_field_access(visitor, node, list, stackframe);break;
        default: {
            compile_error_ast(node, "Unknown node type: %d", node->type);
            break;  
        }
    }

    return node; 


}

// This is the visitor we use when we have a compound statement
AST_t* visit_compound(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* compound = init_ast(COMP_AST);

    compound->stackframe = stackframe;
    stackframe_t *block_frame = NULL;
    if (node->int_value == 1) {
        heap_scope_push(visitor);
        borrow_block_enter(visitor->bctx);
        block_frame = init_stackframe();
        block_frame->stack = list_copy(stackframe->stack);
        block_frame->parent = stackframe;
        block_frame->scope_base = (int)block_frame->stack->size;
        stackframe = block_frame;
    }
    for(int i = 0; i < node->children->size; i++)
    {
        AST_t* child = (AST_t*)node->children->items[i];
        AST_t* new_child = visitor_visit(visitor, child, list, stackframe);
        list_enqueue(compound->children, new_child);
        if (new_child->type == ASSIGNEMENT_AST && !new_child->left)
            list_enqueue(list, new_child);
        else if (new_child->type == FUNC_AST)
            collect_assignments_into_list(new_child, list);
    }
    if (node->int_value == 1) {
        heap_scope_pop_emit(visitor, compound, list, stackframe);
        borrow_block_leave(visitor->bctx);
        /* Stack slots share name pointers with the parent frame; keep block_frame alive for AST. */
        list_free_shallow(block_frame->stack);
        block_frame->stack = 0;
    }
    if (compound->children->size == 1) {
        compound->stack_index = ((AST_t*)compound->children->items[0])->stack_index;
    }
    return compound;
}

// this is the visitor we use when we have an assignent going on like a = 5;. This is pretty much copying everything over
static const char* datatype_name(int dt)
{
    if (IS_ARRAY_TYPE(dt)) return "Array";
    if (IS_INTERFACE_TYPE(dt)) {
        interface_type_t *it = interface_get(INTERFACE_TYPE_ID(dt));
        return it && it->name ? it->name : "interface";
    }
    if (IS_CUST_TYPE(dt)) {
        cust_type_t *t = cust_get(CUST_TYPE_ID(dt));
        return t && t->name ? t->name : "cust";
    }
    switch(dt) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        case TYPE_ADR: return "adr";
        case TYPE_ADR_SHREF: return "&adr";
        case TYPE_ADR_MUTREF: return "&mut adr";
        default: return "unknown";
    }
}

AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* variable = init_ast(ASSIGNEMENT_AST);
    variable->name = node->name;
    variable->datatype = node->datatype;
    variable->op = node->op;

    if (node->left && node->left->type == FIELD_ACCESS_AST) {
        variable->left = visitor_visit(visitor, node->left, list, stackframe);
        variable->parent = visitor_visit(visitor, node->parent, list, stackframe);
        variable->datatype = variable->left->datatype;
        variable->stackframe = stackframe;
        if (variable->left->datatype == TYPE_ADR && variable->parent) {
            AST_t *rhs = variable->parent;
            if (rhs->type == COMP_AST && rhs->children && rhs->children->size == 1)
                rhs = (AST_t *)rhs->children->items[0];
            borrow_handle_field_adr_assign(visitor, variable->left, rhs, list);
        }
        return variable;
    }

    if (node->parent)
    {
        if (IS_CUST_TYPE(variable->datatype) && node->parent->type == CUST_INIT_AST &&
            node->parent->int_value < 0)
            node->parent->int_value = CUST_TYPE_ID(variable->datatype);
        variable->parent = visitor_visit(visitor, node->parent, list, stackframe);
    }

    /* += and -= require an existing variable; find its stack slot from stackframe */
    if (variable->op == PLUS_EQUALS_TOKEN || variable->op == MINUS_EQUALS_TOKEN) {
        int found = 0;
        for (int i = 0; i < (int)stackframe->stack->size; i++) {
            char* var_name = (char*)stackframe->stack->items[i];
            if (var_name && strcmp(var_name, node->name) == 0) {
                variable->stack_index = i + 1;
                variable->stackframe = stackframe;
                variable->datatype = TYPE_INT;
                found = 1;
                break;
            }
        }
        if (!found) {
            compile_error_ast(node, "variable '%s' not defined for += or -=\n", node->name);
        }
        return variable;
    }

    /* Reassignments to an existing variable reuse its stack slot and type. */
    if (variable->op == EQUALS_TOKEN && node->name) {
        stackframe_t *owner_frame = stackframe;
        int existing = stack_lookup_for_name(stackframe, node->name, &owner_frame);
        if (existing != 0 && !node->int_value && variable->datatype == TYPE_UNKNOWN) {
            variable->stack_index = existing;
            variable->stackframe = owner_frame;
            for (int j = (int)list->size - 1; j >= 0; j--) {
                AST_t *def = (AST_t *)list->items[j];
                if (def->type == ASSIGNEMENT_AST && def->name &&
                    strcmp(def->name, node->name) == 0) {
                    variable->datatype = def->datatype;
                    break;
                }
            }
            if (!variable->datatype || variable->datatype == TYPE_UNKNOWN)
                variable->datatype = TYPE_INT;
            if (variable->datatype == TYPE_ADR)
                borrow_handle_owner_reassignment(visitor->bctx, variable, list);
            else if (variable->datatype == TYPE_ADR_SHREF || variable->datatype == TYPE_ADR_MUTREF)
                borrow_handle_loan_assignment(visitor->bctx, variable, list);
            list_enqueue(list, variable);
            return variable;
        }
    }

    /* Type mismatch checks (only for unambiguous literal/array mismatches) */
    if (variable->parent && variable->datatype != TYPE_UNKNOWN) {
        AST_t* rhs = variable->parent;
        int lhs_is_arr = IS_ARRAY_TYPE(variable->datatype);

        if (rhs->type == ARRAY_LITERAL_AST && !lhs_is_arr) {
            compile_error_ast(node, "Cannot assign array literal to %s variable '%s'",
                    datatype_name(variable->datatype), variable->name);
        }
        if (lhs_is_arr && (rhs->type == INT_AST || rhs->type == STRING_AST)) {
            compile_error_ast(node, "Cannot assign %s to Array variable '%s'",
                    rhs->type == INT_AST ? "int" : "str", variable->name);
        }
        if (variable->datatype == TYPE_INT && rhs->type == STRING_AST) {
            compile_error_ast(node, "Cannot assign str to int variable '%s'", variable->name);
        }
        if (variable->datatype == TYPE_STR && rhs->type == INT_AST) {
            compile_error_ast(node, "Cannot assign int to str variable '%s'", variable->name);
        }
        if (variable->datatype == TYPE_ADR && rhs->type == STRING_AST) {
            compile_error_ast(node, "Cannot assign str to adr variable '%s'", variable->name);
        }
        if (variable->datatype == TYPE_ADR && rhs->type == INT_AST) {
            compile_error_ast(node, "Cannot assign int to adr variable '%s'", variable->name);
        }
        if (variable->datatype == TYPE_STR && rhs->type == VAR_AST && rhs->datatype == TYPE_ADR) {
            compile_error_ast(node, "Cannot assign adr to str variable '%s'", variable->name);
        }
        if (IS_CUST_TYPE(variable->datatype) && rhs->type == CUST_INIT_AST) {
            if (rhs->int_value != CUST_TYPE_ID(variable->datatype)) {
                compile_error_ast(node, "cust initializer type mismatch for '%s'", variable->name);
            }
        } else if (IS_CUST_TYPE(variable->datatype) && rhs->type == VAR_AST &&
                   IS_CUST_TYPE(rhs->datatype) &&
                   rhs->datatype != variable->datatype) {
            compile_error_ast(node, "Cannot assign mismatched cust types for '%s'", variable->name);
        }
    }

    if (IS_CUST_TYPE(variable->datatype)) {
        int slots = cust_type_slots(CUST_TYPE_ID(variable->datatype));
        for (int i = 0; i < slots - 1; i++)
            list_enqueue(stackframe->stack, mkstr("0"));
    }

    list_enqueue(stackframe->stack, variable->name);
    variable->stack_index = stackframe->stack->size;
    variable->stackframe = stackframe;
    list_enqueue(list, variable);

    if (!tag_heap_cust_from_rent(variable, variable->parent) && variable->datatype == TYPE_ADR)
        variable->int_value = HEAP_CUST_TAG_NONE;
    if (IS_HEAP_CUST_VAR(variable->datatype, variable->int_value))
        heap_scope_register(visitor, variable->name, variable->int_value,
                            variable->stack_index, variable->stackframe);
    if (IS_HEAP_CUST_VAR(variable->datatype, variable->int_value))
        borrow_register_cust_adr_fields(visitor->bctx, variable->name, variable->int_value);

    if (variable->datatype == TYPE_ADR) {
        borrow_declare_owner(visitor->bctx, variable->name);
        if (variable->parent && variable->parent->type == VAR_AST &&
            is_adr_var(variable->parent, list)) {
            borrow_check_can_move(visitor->bctx, variable->parent->name, " (moved on assign)");
            borrow_mark_moved(visitor->bctx, variable->parent->name);
        }
    } else if (variable->datatype == TYPE_ADR_SHREF || variable->datatype == TYPE_ADR_MUTREF) {
        borrow_handle_loan_assignment(visitor->bctx, variable, list);
    } else if (IS_CUST_TYPE(variable->datatype)) {
        borrow_register_cust_adr_fields(visitor->bctx, variable->name,
                                        CUST_TYPE_ID(variable->datatype));
    }

    return variable;
}

// This is the visitor we use when we have a variable
AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->name && strcmp(node->name, "super") == 0)
        return node;

    int index = stack_index_for_name(stackframe, node->name);

    if (index == 0) {
        compile_error_ast(node, "Undefined variable '%s'\n", node->name);
    }

    node->stack_index = index;
    node->stackframe = stackframe;
    /* Latest matching declaration wins (inner scopes after outer on list) */
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t* def = (AST_t*)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name && node->name &&
            strcmp(def->name, node->name) == 0) {
            node->datatype = def->datatype;
            if (IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                node->int_value = def->int_value;
            break;
        }
    }

    /* Function parameters are not always on the module list (methods use ASSIGNEMENT_AST). */
    if ((!node->datatype || node->datatype == TYPE_UNKNOWN) && node->name) {
        AST_t *fn = visitor->current_func;
        if (!fn && stackframe && visitor->object) {
            for (unsigned int fi = 0; fi < visitor->object->children->size; fi++) {
                AST_t *cand = (AST_t *)visitor->object->children->items[fi];
                if (cand->type == FUNC_AST && cand->stackframe == stackframe) {
                    fn = cand;
                    break;
                }
            }
        }
        if (fn) {
            for (unsigned int pi = 0; pi < fn->children->size; pi++) {
                AST_t *param = (AST_t *)fn->children->items[pi];
                if ((param->type == VAR_AST || param->type == ASSIGNEMENT_AST) &&
                    param->name && strcmp(param->name, node->name) == 0 && param->datatype) {
                    node->datatype = param->datatype;
                    if (IS_HEAP_CUST_VAR(param->datatype, param->int_value))
                        node->int_value = param->int_value;
                    else if (param->datatype == TYPE_ADR && param->int_value >= 0)
                        node->int_value = param->int_value;
                    if (param->datatype == TYPE_ADR && param->multiplier >= INTERFACE_PARAM_MARK)
                        node->multiplier = param->multiplier;
                    break;
                }
            }
        }
    }

    if (node->datatype == TYPE_ADR) {
        borrow_check_use(visitor->bctx, node->name, "");
        borrow_check_owner_frozen(visitor->bctx, node->name, " (owner use while mut borrowed)");
    } else if (node->datatype == TYPE_ADR_SHREF || node->datatype == TYPE_ADR_MUTREF) {
        const char *owner = borrow_loan_owner(visitor->bctx, node->name);
        if (!owner) {
            compile_error_ast(node, "Borrow error: use of expired loan '%s'\n", node->name);
        }
        borrow_check_use(visitor->bctx, owner, " (use through expired loan)");
    }

    return node;
}

static int heap_cust_id_from_var(AST_t *expr, dynamic_list_t *list)
{
    if (!expr)
        return HEAP_CUST_TAG_NONE;
    if (expr->type == VAR_AST && IS_HEAP_CUST_VAR(expr->datatype, expr->int_value))
        return expr->int_value;
    if (expr->type == VAR_AST && expr->name) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, expr->name) == 0 &&
                IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                return def->int_value;
        }
    }
    return HEAP_CUST_TAG_NONE;
}

static int expr_cust_type_id(AST_t *expr, dynamic_list_t *list, stackframe_t *stackframe,
                              visitor_t *visitor)
{
    (void)stackframe;
    if (!expr)
        return -1;
    if (expr->type == VAR_AST && expr->name && strcmp(expr->name, "self") == 0 &&
        visitor && visitor->cust_access_type >= 0)
        return visitor->cust_access_type;
    if (expr->datatype && IS_CUST_TYPE(expr->datatype))
        return CUST_TYPE_ID(expr->datatype);
    int heap_id = heap_cust_id_from_var(expr, list);
    if (heap_id >= 0)
        return heap_id;
    if (expr->type == VAR_AST && expr->name) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, expr->name) == 0) {
                if (IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                    return def->int_value;
                if (IS_CUST_TYPE(def->datatype))
                    return CUST_TYPE_ID(def->datatype);
            }
        }
    }
    return -1;
}

static int tag_heap_cust_from_rent(AST_t *variable, AST_t *rhs)
{
    if (!variable || !rhs)
        return 0;
    if (rhs->type == COMP_AST && rhs->children && rhs->children->size == 1)
        rhs = (AST_t *)rhs->children->items[0];
    if (rhs->type != CALL_AST || !rhs->name)
        return 0;
    if (strcmp(rhs->name, "rentMul") != 0 && strcmp(rhs->name, "rent") != 0)
        return 0;
    if (!rhs->parent || rhs->parent->children->size < 2)
        return 0;
    AST_t *ty = (AST_t *)rhs->parent->children->items[1];
    int ty_dt = ty ? ty->datatype : TYPE_UNKNOWN;
    if (ty && ty->type == TYPE_SIZE_AST && ty->multiplier)
        ty_dt = ty->multiplier;
    if (!ty || !IS_CUST_TYPE(ty_dt))
        return 0;
    variable->datatype = TYPE_ADR;
    variable->int_value = CUST_TYPE_ID(ty_dt);
    rhs->int_value = variable->int_value;
    return 1;
}

// This is the visitor we use when we have a function definition
AST_t* visit_func(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* func = init_ast(FUNC_AST);
    func->name = node->name;
    func->datatype = node->datatype;
    func->children =  init_list(sizeof(struct AST_S*));

    stackframe_t * new_stackframe = init_stackframe();
    list_enqueue(new_stackframe->stack, 0);
    list_enqueue(new_stackframe->stack, 0);

    borrow_func_enter(visitor->bctx);

    int saved_access = visitor->cust_access_type;
    AST_t *saved_func = visitor->current_func;
    visitor->current_func = node;
    if (node->int_value >= 0 && node->id)
        visitor->cust_access_type = node->int_value;

    /* Register parameter names on the stack frame before visiting the body. */
    for (int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        if (IS_INTERFACE_TYPE(child->datatype)) {
            child->multiplier = INTERFACE_PARAM_MARK + INTERFACE_TYPE_ID(child->datatype);
            child->datatype = TYPE_ADR;
        }
        if (child->name && strcmp(child->name, "self") == 0 && node->int_value >= 0) {
            child->datatype = TYPE_ADR;
            child->int_value = node->int_value;
        }
        if ((child->type == VAR_AST || child->type == ASSIGNEMENT_AST) && child->name) {
            if (child->name && strcmp(child->name, "self") == 0 && node->int_value >= 0) {
                list_enqueue(new_stackframe->stack, child->name);
                child->stack_index = (int)new_stackframe->stack->size;
                child->stackframe = new_stackframe;
                if (child->datatype == TYPE_ADR)
                    borrow_declare_owner(visitor->bctx, child->name);
                continue;
            }
            if (IS_CUST_TYPE(child->datatype)) {
                int slots = cust_type_slots(CUST_TYPE_ID(child->datatype));
                for (int s = 0; s < slots - 1; s++)
                    list_enqueue(new_stackframe->stack, mkstr("0"));
            }
            list_enqueue(new_stackframe->stack, child->name);
            child->stack_index = (int)new_stackframe->stack->size;
            child->stackframe = new_stackframe;
            if (child->datatype == TYPE_ADR)
                borrow_declare_owner(visitor->bctx, child->name);
        }
    }

    for(int i = 0; i < node->children->size; i++)
    {
        AST_t* child = (AST_t*)node->children->items[i];
        AST_t* new_child = visitor_visit(visitor, child, list, new_stackframe);
        list_enqueue(func->children, new_child);
    }

    func->parent = visitor_visit(visitor, node->parent, list, new_stackframe);
    func->stackframe = new_stackframe;

    borrow_func_leave(visitor->bctx);
    visitor->cust_access_type = saved_access;
    visitor->current_func = saved_func;

    return func;
}


// This is the part of the visitor when we want to handle a function call
AST_t* visit_caller(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* variable = 0;
    dynamic_list_t *arguments = init_list(sizeof(struct AST_S));

    if (node->left) {
        AST_t *recv = visitor_visit(visitor, node->left, list, stackframe);
        int is_super = recv->type == VAR_AST && recv->name &&
                       strcmp(recv->name, "super") == 0;
        int cust_id;
        if (is_super) {
            if (visitor->cust_access_type < 0) {
                compile_error_ast(node, "'super' used outside of method context\n");
            }
            cust_id = visitor->cust_access_type;
        } else {
            cust_id = expr_cust_type_id(recv, list, stackframe, visitor);
        }
        int iface_id = -1;
        if (cust_id < 0 && recv->type == VAR_AST && recv->datatype == TYPE_ADR &&
            recv->multiplier >= INTERFACE_PARAM_MARK) {
            iface_id = (int)recv->multiplier - INTERFACE_PARAM_MARK;
        }
        if (cust_id < 0 && iface_id < 0) {
            compile_error_ast(node, "method call requires cust receiver\n");
        }
        cust_method_t *method = 0;
        if (iface_id >= 0) {
            interface_method_t *im = interface_method_by_name(iface_id, node->name);
            if (!im) {
                compile_error_ast(node, "method '%s' not found on interface\n", node->name);
            }
            node->multiplier = CUST_CALL_VIRTUAL;
            node->int_value = im->vtable_slot;
            node->id = 1;
            node->datatype = im->return_type;
        } else {
        int resolve_id = is_super ? cust_base_type_id(cust_id) : cust_id;
        if (is_super && resolve_id < 0) {
            compile_error_ast(node, "'super' used in type without base class\n");
        }
        method = cust_method_lookup(resolve_id, node->name);
        if (!method) {
            compile_error_ast(node, "method '%s' not found on cust type\n", node->name);
        }
        if (method->visibility == CUST_VIS_PRIVATE &&
            visitor->cust_access_type != resolve_id) {
            compile_error_ast(node, "private method '%s' is not accessible here\n", node->name);
        }
        if (is_super) {
            AST_t *self_ast = init_ast(VAR_AST);
            self_ast->name = mkstr("self");
            recv = visit_var(visitor, self_ast, list, stackframe);
            recv->datatype = TYPE_ADR;
            recv->int_value = visitor->cust_access_type;
            node->multiplier = CUST_CALL_SUPER;
            node->name = mkstr(method->mangled_name);
            node->id = 1;
        } else if (method->is_virtual && (IS_HEAP_CUST_VAR(recv->datatype, recv->int_value) ||
                                          recv->datatype == TYPE_ADR)) {
            node->multiplier = CUST_CALL_VIRTUAL;
            node->int_value = method->vtable_slot;
            node->id = 1;
            node->datatype = method->return_type;
        } else {
            if (strcmp(node->name, "drop") == 0 && recv->type == VAR_AST && recv->name &&
                IS_HEAP_CUST_VAR(recv->datatype, recv->int_value))
                heap_scope_mark_dropped(visitor, recv->name);
            node->name = mkstr(method->mangled_name);
            node->id = 1;
        }
        }
        AST_t *args = init_ast(COMP_AST);
        list_enqueue(args->children, recv);
        for (int i = 0; i < node->parent->children->size; i++) {
            AST_t *child = (AST_t *)node->parent->children->items[i];
            list_enqueue(args->children, visitor_visit(visitor, child, list, stackframe));
        }
        node->parent = args;
        if (node->multiplier != CUST_CALL_VIRTUAL)
            variable = variable_lookup(visitor->object->children, node->name);
        if (node->multiplier != CUST_CALL_VIRTUAL && !variable) {
            compile_error_ast(node, "undefined method '%s'\n", node->name);
        }
    } else {
        for (int i = 0; i < node->parent->children->size; i++) {
            AST_t *child = (AST_t *)node->parent->children->items[i];
            list_enqueue(arguments, visitor_visit(visitor, child, list, stackframe));
        }
        variable = variable_lookup(visitor->object->children, node->name);
    }

    if(variable)
    {
        check_arguments(node, variable);
        if (node->parent && node->parent->children) {
            for (unsigned int ai = 0; ai < node->parent->children->size; ai++) {
                AST_t *param = ai < variable->children->size
                    ? (AST_t *)variable->children->items[ai]
                    : 0;
                if (!param || param->multiplier < INTERFACE_PARAM_MARK)
                    continue;
                int iid = (int)param->multiplier - INTERFACE_PARAM_MARK;
                AST_t *arg = (AST_t *)node->parent->children->items[ai];
                int cust_id = heap_cust_id_from_var(arg, list);
                if (cust_id < 0) {
                    compile_error_ast(node,
                                      "interface argument requires heap object handle\n");
                }
                if (!interface_cust_implements(cust_id, iid)) {
                    compile_error_ast(node,
                                      "argument type does not implement required interface\n");
                }
            }
        }
        node->datatype = variable->datatype;
    }
    else if (strcmp(node->name, "rent") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "rentMul") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "PeekByte") == 0 || strcmp(node->name, "PeekInt") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "AdrLo") == 0 || strcmp(node->name, "AdrHi") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "moveOut") == 0 || strcmp(node->name, "PokeByte") == 0 ||
             strcmp(node->name, "PokeInt") == 0 || strcmp(node->name, "AddInt") == 0 ||
             strcmp(node->name, "Memcpy") == 0 || strcmp(node->name, "Memset") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "RentGrow") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "timer") == 0 || strcmp(node->name, "join") == 0)
        node->datatype = TYPE_INT;

    if (node->parent && node->parent->children->size > 0) {
        AST_t *arg0 = (AST_t*)node->parent->children->items[0];
        unsigned int nargs = node->parent->children->size;
        if (strcmp(node->name, "rent") == 0 && nargs == 2)
            node->name = mkstr("rentMul");
        if (strcmp(node->name, "rentMul") == 0 && nargs >= 2) {
            AST_t *ty = (AST_t *)node->parent->children->items[1];
            int ty_dt = ty ? ty->datatype : TYPE_UNKNOWN;
            if (ty && ty->type == TYPE_SIZE_AST && ty->multiplier)
                ty_dt = ty->multiplier;
            if (ty && IS_CUST_TYPE(ty_dt))
                node->int_value = CUST_TYPE_ID(ty_dt);
        }
        if (strcmp(node->name, "moveOut") == 0) {
            if (!IS_ADR_LIKE(arg0->datatype)) {
                compile_error_ast(node, "Borrow error: moveOut requires adr\n");
            }
            borrow_check_can_move_expr(visitor->bctx, arg0, list, " (already moved)");
            char *key = borrow_owner_key_from_expr(arg0, list);
            if (key) {
                borrow_mark_moved(visitor->bctx, key);
                free(key);
            }
        } else if (strcmp(node->name, "PeekByte") == 0 || strcmp(node->name, "PeekInt") == 0 ||
                   strcmp(node->name, "AdrLo") == 0 || strcmp(node->name, "AdrHi") == 0) {
            borrow_check_adr_expr(visitor->bctx, arg0, list, " (heap op on moved adr)");
        } else if (strcmp(node->name, "PokeByte") == 0 || strcmp(node->name, "PokeInt") == 0 ||
                   strcmp(node->name, "AddInt") == 0 || strcmp(node->name, "Memset") == 0) {
            borrow_check_adr_expr(visitor->bctx, arg0, list, " (heap op on moved adr)");
            char *key = borrow_owner_key_from_expr(arg0, list);
            if (key) {
                borrow_check_owner_writable(visitor->bctx, key, " (write through owner while borrowed)");
                free(key);
            }
        } else if (strcmp(node->name, "Memcpy") == 0) {
            borrow_check_adr_expr(visitor->bctx, arg0, list, " (memcpy dst on moved adr)");
            char *dk = borrow_owner_key_from_expr(arg0, list);
            if (dk) {
                borrow_check_owner_writable(visitor->bctx, dk, " (write through owner while borrowed)");
                free(dk);
            }
            if (nargs > 1) {
                AST_t *arg1 = (AST_t*)node->parent->children->items[1];
                borrow_check_adr_expr(visitor->bctx, arg1, list, " (memcpy src on moved adr)");
            }
        } else if (strcmp(node->name, "RentGrow") == 0) {
            if (!IS_ADR_LIKE(arg0->datatype)) {
                compile_error_ast(node, "Borrow error: RentGrow requires adr\n");
            }
            borrow_check_can_move_expr(visitor->bctx, arg0, list, " (already moved)");
            char *key = borrow_owner_key_from_expr(arg0, list);
            if (key) {
                borrow_mark_moved(visitor->bctx, key);
                free(key);
            }
        }
    }

    node->stackframe = stackframe;

    return node;
}

AST_t* visit_int(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_type_size(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->int_value <= 0) {
        compile_error_ast(node, "sizeof unsupported or zero-size type\n");
    }
    node->multiplier = node->datatype;
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_str(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    dynamic_list_t * string = str_to_hex_list(node->string_value);

    list_enqueue(stackframe->stack, 0);
    node->stack_index = stackframe->stack->size;

    return node;
}

static int is_string_node(AST_t* node, dynamic_list_t* list)
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

AST_t* visit_binop(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t * binop = init_ast(BINOP_AST);
    binop->left = visitor_visit(visitor, node->left, list, stackframe);
    binop->right = visitor_visit(visitor, node->right, list, stackframe);
    binop->op = node->op;
    if (node->op == PLUS_TOKEN && is_string_node(binop->left, list) && is_string_node(binop->right, list))
        binop->datatype = TYPE_STR;
    else if (node->op == DEQUALS_TOKEN || node->op == NOT_EQUALS_TOKEN || node->op == LT_TOKEN
             || node->op == GT_TOKEN || node->op == LTE_TOKEN || node->op == GTE_TOKEN
             || node->op == AND_TOKEN || node->op == OR_TOKEN)
        binop->datatype = TYPE_BOOL;
    list_enqueue(stackframe->stack, mkstr("0"));
    binop->stack_index = stackframe->stack->size;
    binop->stackframe = stackframe;
    return binop;
}

AST_t* visit_return(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* return_node = init_ast(RETURN_AST);
    return_node->parent = visitor_visit(visitor, node->parent, list, stackframe);
    if (return_node->parent)
        borrow_check_adr_expr(visitor->bctx, return_node->parent, list, " (return of moved adr)");
    return return_node;
}

AST_t* visit_access(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->parent) {
        node->parent = visitor_visit(visitor, node->parent, list, stackframe);
        node->int_value = node->parent->stack_index;
        node->stackframe = stackframe;
        if (node->left)
            node->left = visitor_visit(visitor, node->left, list, stackframe);
        list_enqueue(stackframe->stack, mkstr("0"));
        node->stack_index = stackframe->stack->size;
        if (IS_ARRAY_TYPE(node->parent->datatype))
            node->datatype = ARRAY_ELEM_TYPE(node->parent->datatype);
        else if (node->parent->datatype == TYPE_ADR)
            node->datatype = TYPE_INT;
        return node;
    }

    /* Look up base variable index before visiting index expr (which adds to stack) */
    int base_index = stack_index_for_name(stackframe, node->name);
    if (base_index == 0 && node->name) {
        compile_error_ast(node, "Undefined variable '%s' in index access\n", node->name);
    }
    node->int_value = base_index;
    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t* def = (AST_t*)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name && node->name &&
            strcmp(def->name, node->name) == 0) {
            if (def->datatype == TYPE_ADR) {
                node->datatype = TYPE_INT;
                borrow_check_use(visitor->bctx, node->name, " (index on moved adr)");
                borrow_check_owner_frozen(visitor->bctx, node->name,
                                          " (index on owner while mut borrowed)");
            } else if (IS_ARRAY_TYPE(def->datatype))
                node->datatype = ARRAY_ELEM_TYPE(def->datatype);
            break;
        }
    }
    return node;
}

AST_t* visit_slice(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* base = (AST_t*)node->children->items[0];
    AST_t* start_expr = (AST_t*)node->children->items[1];
    AST_t* end_expr = (AST_t*)node->children->items[2];
    /* Visit base first to get correct stack_index (before start/end add to stack) */
    visitor_visit(visitor, base, list, stackframe);
    int base_index = base->stack_index;
    if (base_index == 0 && base->name) {
        compile_error_ast(node, "Undefined variable '%s' in slice\n", base->name);
    }
    visitor_visit(visitor, start_expr, list, stackframe);
    visitor_visit(visitor, end_expr, list, stackframe);
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->int_value = base_index;
    return node;
}

AST_t* visit_array_literal(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    int count = node->children->size;
    for (int i = 0; i < count; i++) {
        AST_t* elem = (AST_t*)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, elem, list, stackframe);
    }
    node->stack_index = ((AST_t*)node->children->items[0])->stack_index;
    node->stackframe = stackframe;
    node->int_value = count;
    return node;
}

AST_t* visit_bool(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_BOOL;
    return node;
}

static AST_t* visit_dupe(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);
    if (node->left)
        borrow_check_adr_expr(visitor->bctx, node->left, list, " (passed to dupe)");
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_if(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    borrow_snapshot_t *pre = borrow_snapshot_take(visitor->bctx);

    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);

    borrow_block_enter(visitor->bctx);
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t* child = (AST_t*)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }
    borrow_block_leave(visitor->bctx);

    borrow_snapshot_t *then_snap = borrow_snapshot_take(visitor->bctx);
    borrow_snapshot_restore(visitor->bctx, pre);

    if (node->right) {
        borrow_block_enter(visitor->bctx);
        node->right = visitor_visit(visitor, node->right, list, stackframe);
        borrow_block_leave(visitor->bctx);
    }

    borrow_snapshot_t *else_snap = node->right
        ? borrow_snapshot_take(visitor->bctx)
        : pre;
    borrow_snapshot_merge(visitor->bctx, then_snap, else_snap);
    borrow_snapshot_free(pre);
    borrow_snapshot_free(then_snap);
    if (node->right)
        borrow_snapshot_free(else_snap);

    node->stackframe = stackframe;
    return node;
}

AST_t* visit_unary(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* unary = init_ast(UNARY_AST);
    unary->left = visitor_visit(visitor, node->left, list, stackframe);
    unary->op = node->op;
    list_enqueue(stackframe->stack, mkstr("0"));
    unary->stack_index = stackframe->stack->size;
    unary->stackframe = stackframe;
    if (node->op == NOT_TOKEN)
        unary->datatype = TYPE_BOOL;
    return unary;
}

AST_t* visit_for_clause(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t* child = (AST_t*)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }
    node->stackframe = stackframe;
    return node;
}

AST_t* visit_loop_until(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);
    borrow_block_enter(visitor->bctx);
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t* child = (AST_t*)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }
    borrow_block_leave(visitor->bctx);
    node->stackframe = stackframe;
    return node;
}

AST_t* visit_inc_dec(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    node->left = visitor_visit(visitor, node->left, list, stackframe);
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_cust_def(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    (void)list;
    (void)stackframe;
    if (node->int_value == -1 || node->int_value == -2)
        return node;

    node->int_value = cust_lookup_by_name(node->name);
    if (node->int_value < 0) {
        compile_error_ast(node, "cust type '%s' not registered\n", node->name);
    }

    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        if (child->type != FUNC_AST)
            continue;
        child->int_value = node->int_value;
        child->id = 1;
        for (int p = 0; p < child->children->size; p++) {
            AST_t *param = (AST_t *)child->children->items[p];
            if (param->name && strcmp(param->name, "self") == 0) {
                param->datatype = TYPE_ADR;
                param->int_value = node->int_value;
            }
        }
        AST_t *method = visit_func(visitor, child, list, stackframe);
        node->children->items[i] = method;
        list_enqueue(visitor->object->children, method);
    }

    node->datatype = MAKE_CUST_TYPE(node->int_value);
    return node;
}

AST_t* visit_cust_init(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    (void)visitor;
    if (node->int_value < 0) {
        compile_error_ast(node, "cust initializer requires a typed variable or Type{...} syntax\n");
    }
    cust_type_t *type = cust_get(node->int_value);
    if (!type) {
        compile_error_ast(node, "unknown cust type in initializer\n");
    }

    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *entry = (AST_t *)node->children->items[i];
        cust_field_t *field = cust_field_by_name(node->int_value, entry->name);
        if (!field) {
            compile_error_ast(node, "unknown field '%s' in cust '%s' initializer",
                    entry->name, type->name);
        }
        entry->parent = visitor_visit(visitor, entry->parent, list, stackframe);
        entry->int_value = field->offset;
        entry->datatype = field->datatype;
    }

    list_enqueue(stackframe->stack, mkstr("0"));
    int slots = cust_type_slots(node->int_value);
    for (int i = 1; i < slots; i++)
        list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = MAKE_CUST_TYPE(node->int_value);
    return node;
}

AST_t* visit_field_access(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    node->left = visitor_visit(visitor, node->left, list, stackframe);

    int cust_id = -1;
    int parent_offset = 0;
    int declaring_id = -1;
    AST_t *container = node->left;

    if (container->type == VAR_AST) {
        if (container->name && strcmp(container->name, "self") == 0 &&
            visitor->cust_access_type >= 0) {
            if (container->datatype == TYPE_UNKNOWN)
                container->datatype = TYPE_ADR;
            container->int_value = visitor->cust_access_type;
        }
        int dt = container->datatype;
        if (!IS_CUST_TYPE(dt) && !IS_HEAP_CUST_VAR(dt, container->int_value))
            dt = datatype_of_var_name(list, container->name);
        if (IS_HEAP_CUST_VAR(dt, container->int_value)) {
            cust_id = container->int_value;
            container->stack_index = stack_index_for_name(stackframe, container->name);
            if (container->stack_index == 0) {
                compile_error_ast(node, "undefined variable '%s' in field access",
                        container->name ? container->name : "?");
            }
            node->multiplier = CUST_ACCESS_HEAP;
            node->id = container->stack_index;
        } else if (!IS_CUST_TYPE(dt)) {
            compile_error_ast(node, "field access on non-cust variable '%s'",
                    container->name ? container->name : "?");
        } else {
            cust_id = CUST_TYPE_ID(dt);
            container->stack_index = stack_index_for_name(stackframe, container->name);
            if (container->stack_index == 0) {
                compile_error_ast(node, "undefined variable '%s' in field access", container->name);
            }
        }
    } else if (container->type == FIELD_ACCESS_AST) {
        if (container->multiplier == CUST_ACCESS_HEAP) {
            if (!IS_CUST_TYPE(container->datatype)) {
                compile_error_ast(node, "nested heap field access requires cust field type");
            }
            cust_id = CUST_TYPE_ID(container->datatype);
            parent_offset = container->int_value;
            node->multiplier = CUST_ACCESS_HEAP;
            node->id = container->id;
        } else {
            if (!IS_CUST_TYPE(container->datatype)) {
                compile_error_ast(node, "nested field access requires cust field type");
            }
            cust_id = CUST_TYPE_ID(container->datatype);
            parent_offset = container->int_value;
        }
    } else {
        compile_error_ast(node, "invalid base for field access");
    }

    cust_field_t *field = cust_field_lookup(cust_id, node->name, &declaring_id);
    if (!field) {
        cust_method_t *method = cust_method_lookup(cust_id, node->name);
        if (method) {
            compile_error_ast(node, "'%s' is a method; call it with ()", node->name);
        } else {
            compile_error_ast(node, "field '%s' not found in cust type", node->name);
        }
    }

    if (field->visibility == CUST_VIS_PRIVATE && visitor->cust_access_type != declaring_id) {
        compile_error_ast(node, "private field '%s' is not accessible here", node->name);
    }

    node->datatype = field->datatype;
    if (node->multiplier == CUST_ACCESS_HEAP)
        node->int_value = parent_offset + field->offset + cust_heap_header_size(cust_id);
    else
        node->int_value = parent_offset + field->offset;
    AST_t *root = field_access_root(node);
    if (root && root->type == VAR_AST && node->multiplier != CUST_ACCESS_HEAP)
        node->id = stack_index_for_name(stackframe, root->name);
    node->stackframe = stackframe;

    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    return node;
}


#include "include/visitor.h"
#include "include/borrow.h"
#include "include/heap_scope.h"
#include "include/token.h"
#include "include/cust.h"
#include "include/types.h"
#include "include/interface.h"
#include "include/module.h"
#include "include/numeric.h"
#include "include/errors.h"
#include "include/constexpr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INTERFACE_PARAM_MARK 2000

static int stack_lookup_for_name(stackframe_t *stackframe, const char *name,
                                 stackframe_t **out_frame);

static int closure_name_in_list(dynamic_list_t *list, const char *name)
{
    if (!list || !name)
        return 0;
    for (unsigned int i = 0; i < list->size; i++) {
        char *n = (char *)list->items[i];
        if (n && strcmp(n, name) == 0)
            return 1;
    }
    return 0;
}

static void closure_list_add_unique(dynamic_list_t *list, const char *name)
{
    if (!name || !name[0] || closure_name_in_list(list, name))
        return;
    list_enqueue(list, mkstr(name));
}

static void closure_collect_var_refs(AST_t *n, dynamic_list_t *uses, int depth)
{
    if (!n || depth > 256)
        return;
    if (n->type == VAR_AST && n->name)
        closure_list_add_unique(uses, n->name);
    if (n->type == FUNC_AST)
        return;
    if (n->children) {
        for (unsigned int i = 0; i < n->children->size; i++)
            closure_collect_var_refs((AST_t *)n->children->items[i], uses, depth + 1);
    }
    if (n->left)
        closure_collect_var_refs(n->left, uses, depth + 1);
    if (n->right)
        closure_collect_var_refs(n->right, uses, depth + 1);
    if (n->type == RETURN_AST && n->parent)
        closure_collect_var_refs(n->parent, uses, depth + 1);
    if (n->type == ASSIGNEMENT_AST && n->parent)
        closure_collect_var_refs(n->parent, uses, depth + 1);
}

static void closure_collect_bound(AST_t *n, dynamic_list_t *bound, int depth)
{
    if (!n || depth > 256)
        return;
    if (n->type == ASSIGNEMENT_AST && n->name)
        closure_list_add_unique(bound, n->name);
    if (n->type == FUNC_AST)
        return;
    if (n->children) {
        for (unsigned int i = 0; i < n->children->size; i++)
            closure_collect_bound((AST_t *)n->children->items[i], bound, depth + 1);
    }
    if (n->left)
        closure_collect_bound(n->left, bound, depth + 1);
    if (n->right)
        closure_collect_bound(n->right, bound, depth + 1);
}

static dynamic_list_t *closure_analyze_captures(AST_t *func_node, stackframe_t *outer_frame)
{
    dynamic_list_t *uses = init_list(sizeof(char *));
    dynamic_list_t *bound = init_list(sizeof(char *));
    dynamic_list_t *captures = init_list(sizeof(char *));

    for (int i = 0; i < func_node->children->size; i++) {
        AST_t *p = (AST_t *)func_node->children->items[i];
        if (p && p->name)
            closure_list_add_unique(bound, p->name);
    }
    if (func_node->parent) {
        closure_collect_bound(func_node->parent, bound, 0);
        closure_collect_var_refs(func_node->parent, uses, 0);
    }

    for (unsigned int i = 0; i < uses->size; i++) {
        char *name = (char *)uses->items[i];
        if (closure_name_in_list(bound, name))
            continue;
        if (strcmp(name, "super") == 0)
            continue;
        if (stack_lookup_for_name(outer_frame, name, NULL) == 0)
            continue;
        closure_list_add_unique(captures, name);
    }

    list_free_shallow(uses);
    list_free_shallow(bound);
    return captures;
}

static void visitor_register_func_param(visitor_t *visitor, AST_t *child,
                                        AST_t *func_node, stackframe_t *new_stackframe)
{
    if (IS_INTERFACE_TYPE(child->datatype)) {
        child->multiplier = INTERFACE_PARAM_MARK + INTERFACE_TYPE_ID(child->datatype);
        child->datatype = TYPE_ADR;
    }
    if (child->name && strcmp(child->name, "self") == 0 && func_node->int_value >= 0) {
        child->datatype = TYPE_ADR;
        child->int_value = func_node->int_value;
    }
    if ((child->type != VAR_AST && child->type != ASSIGNEMENT_AST) || !child->name)
        return;
    if (child->name && strcmp(child->name, "self") == 0 && func_node->int_value >= 0) {
        list_enqueue(new_stackframe->stack, child->name);
        child->stack_index = (int)new_stackframe->stack->size;
        child->stackframe = new_stackframe;
        if (child->datatype == TYPE_ADR)
            borrow_declare_owner(visitor->bctx, child->name);
        return;
    }
    if (func_node->id && func_node->int_value >= 0 && func_node->name &&
        strstr(func_node->name, "operator_") && child->name &&
        strcmp(child->name, "self") != 0 && child->datatype == TYPE_UNKNOWN) {
        child->datatype = MAKE_CUST_TYPE(func_node->int_value);
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

static unsigned int closure_capture_param_count(AST_t *func_ast)
{
    unsigned int count = 0;
    if (!func_ast || !func_ast->left || !func_ast->left->children)
        return 0;
    for (unsigned int i = 0; i < func_ast->left->children->size; i++) {
        AST_t *p = (AST_t *)func_ast->left->children->items[i];
        if (!p || p->multiplier != CLOSURE_CAPTURE_MARK)
            break;
        count++;
    }
    return count;
}

static void visitor_prepend_closure_args(visitor_t *visitor, AST_t *node, AST_t *func_ast,
                                         dynamic_list_t *arguments, dynamic_list_t *list,
                                         stackframe_t *stackframe)
{
    unsigned int cap_count = closure_capture_param_count(func_ast);
    if (!cap_count)
        return;

    dynamic_list_t *params = func_ast->left->children;
    AST_t *new_parent = init_ast(COMP_AST);
    for (unsigned int pi = 0; pi < cap_count; pi++) {
        AST_t *p = (AST_t *)params->items[pi];
        AST_t *cap = init_ast(VAR_AST);
        cap->name = mkstr(p->name);
        cap->source_file = node->source_file;
        cap->source_line = node->source_line;
        list_enqueue(new_parent->children,
                     visitor_visit(visitor, cap, list, stackframe));
    }
    for (unsigned int ai = 0; ai < arguments->size; ai++)
        list_enqueue(new_parent->children, arguments->items[ai]);
    node->parent = new_parent;
}

static AST_t *visitor_unwrap_comp(AST_t *node)
{
    while (node && node->type == COMP_AST && node->children && node->children->size == 1)
        node = (AST_t *)node->children->items[0];
    return node;
}

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

static int array_length_for_var(const char *name, dynamic_list_t *list)
{
    if (!name || !list)
        return -1;
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t *def = (AST_t *)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name &&
            strcmp(def->name, name) == 0 && IS_ARRAY_TYPE(def->datatype) &&
            def->parent && def->parent->type == ARRAY_LITERAL_AST)
            return def->parent->int_value;
    }
    return -1;
}

static void visitor_check_const_index(AST_t *node, const char *arr_name, int index,
                                      dynamic_list_t *list)
{
    int len = array_length_for_var(arr_name, list);
    if (len >= 0 && (index < 0 || index >= len))
        compile_error_ast(node, "array index %d out of bounds for length %d", index, len);
}

static int visitor_infer_datatype_from_rhs(AST_t *rhs)
{
    if (!rhs)
        return TYPE_UNKNOWN;
    if (rhs->type == INT_AST)
        return TYPE_INT;
    if (rhs->type == STRING_AST)
        return TYPE_STR;
    if (rhs->type == BOOL_AST)
        return TYPE_BOOL;
    if (rhs->type == FLOAT_AST)
        return rhs->datatype ? rhs->datatype : TYPE_F64;
    if (rhs->type == VAR_AST && rhs->datatype != TYPE_UNKNOWN)
        return rhs->datatype;
    if (rhs->type == ARRAY_LITERAL_AST && rhs->children && rhs->children->size > 0) {
        AST_t *e0 = (AST_t *)rhs->children->items[0];
        if (e0 && e0->datatype != TYPE_UNKNOWN)
            return TYPE_ARRAY + e0->datatype;
    }
    if (rhs->type == CUST_INIT_AST && rhs->int_value >= 0)
        return MAKE_CUST_TYPE(rhs->int_value);
    return TYPE_UNKNOWN;
}

static AST_t *field_access_root(AST_t *node)
{
    while (node && node->type == FIELD_ACCESS_AST && node->left)
        node = node->left;
    return node;
}

static int visitor_is_compound_assign_op(int op)
{
    return op == PLUS_EQUALS_TOKEN || op == MINUS_EQUALS_TOKEN ||
           op == ASTERISK_EQUALS_TOKEN || op == SLASH_EQUALS_TOKEN ||
           op == MODULUS_EQUALS_TOKEN || op == CARET_EQUALS_TOKEN ||
           op == BITAND_EQUALS_TOKEN || op == BITOR_EQUALS_TOKEN;
}

static cust_field_t *visitor_field_from_access(AST_t *fa, dynamic_list_t *list,
                                               stackframe_t *stackframe)
{
    (void)stackframe;
    if (!fa || fa->type != FIELD_ACCESS_AST || !fa->name)
        return NULL;
    AST_t *container = fa->left;
    int cust_id = -1;
    if (container && container->type == VAR_AST) {
        int dt = container->datatype;
        if (!IS_CUST_TYPE(dt) && !IS_HEAP_CUST_VAR(dt, container->int_value) && container->name)
            dt = datatype_of_var_name(list, container->name);
        if (IS_HEAP_CUST_VAR(dt, container->int_value))
            cust_id = container->int_value;
        else if (IS_CUST_TYPE(dt))
            cust_id = CUST_TYPE_ID(dt);
    } else if (container && container->type == FIELD_ACCESS_AST &&
               IS_CUST_TYPE(container->datatype)) {
        cust_id = CUST_TYPE_ID(container->datatype);
    }
    if (cust_id < 0)
        return NULL;
    return cust_field_by_name(cust_id, fa->name);
}

static int visitor_literal_matches_array_elem(AST_t *elem, int elem_dt)
{
    if (!elem || elem_dt == TYPE_UNKNOWN)
        return 0;
    if (elem->type == INT_AST)
        return IS_INT_TYPE(elem_dt) || elem_dt == TYPE_BOOL;
    if (elem->type == BOOL_AST)
        return elem_dt == TYPE_BOOL || elem_dt == TYPE_INT;
    if (elem->type == FLOAT_AST)
        return IS_FLOAT_TYPE(elem_dt);
    if (elem->type == STRING_AST)
        return elem_dt == TYPE_STR;
    if (elem->type == CUST_INIT_AST && IS_CUST_TYPE(elem_dt))
        return elem->int_value == CUST_TYPE_ID(elem_dt);
    return 0;
}

static void visitor_check_array_literal_types(AST_t *arr, int array_dt, AST_t *site)
{
    if (!arr || arr->type != ARRAY_LITERAL_AST || !IS_ARRAY_TYPE(array_dt))
        return;
    int elem_dt = ARRAY_ELEM_TYPE(array_dt);
    for (unsigned int i = 0; i < arr->children->size; i++) {
        AST_t *elem = (AST_t *)arr->children->items[i];
        if (!visitor_literal_matches_array_elem(elem, elem_dt))
            compile_error_ast(site, "array element type mismatch at index %u", i);
    }
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

static AST_t *function_lookup(visitor_t *visitor, dynamic_list_t *list, char *name)
{
    AST_t *v = variable_lookup(list, name);
    if (v)
        return v;
    return variable_lookup(visitor->object->children, name);
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
        case FLOAT_AST: return visit_float(visitor, node, list, stackframe);break;
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
        case BREAK_AST: return visit_break(visitor, node, list, stackframe);break;
        case CONTINUE_AST: return visit_continue(visitor, node, list, stackframe);break;
        case SWITCH_AST: return visit_switch(visitor, node, list, stackframe);break;
        case CASE_AST: return visit_case(visitor, node, list, stackframe);break;
        case FOREACH_AST: return visit_foreach(visitor, node, list, stackframe);break;
        case ENUM_AST: return visit_enum(visitor, node, list, stackframe);break;
        case INC_DEC_AST: return visit_inc_dec(visitor, node, list, stackframe);break;
        case DUPE_AST: return visit_dupe(visitor, node, list, stackframe);break;
        case TYPE_SIZE_AST: return visit_type_size(visitor, node, list, stackframe);break;
        case CUST_DEF_AST: return visit_cust_def(visitor, node, list, stackframe);break;
        case CUST_INIT_AST: return visit_cust_init(visitor, node, list, stackframe);break;
        case FIELD_ACCESS_AST: return visit_field_access(visitor, node, list, stackframe);break;
        case TRY_STMT_AST: return visit_try(visitor, node, list, stackframe);break;
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
    if (IS_NUMERIC_TYPE(dt))
        return numeric_type_name(dt);
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

static void visitor_enqueue_stack_slots(stackframe_t *stackframe, int slots)
{
    for (int i = 0; i < slots; i++)
        list_enqueue(stackframe->stack, mkstr("0"));
}

static int visitor_numeric_assign_ok(int dst_dt, AST_t *rhs)
{
    if (!rhs)
        return 0;
    int rhs_dt = rhs->datatype;
    if (rhs->type == INT_AST)
        rhs_dt = TYPE_INT;
    else if (rhs->type == FLOAT_AST)
        rhs_dt = TYPE_F64;
    else if (rhs->type == BOOL_AST)
        rhs_dt = TYPE_INT;
    if (rhs_dt == TYPE_UNKNOWN)
        return 1;
    if (IS_FLOAT_TYPE(dst_dt)) {
        if (IS_FLOAT_TYPE(rhs_dt))
            return numeric_float_level(rhs_dt) <= numeric_float_level(dst_dt);
        if (IS_INT_TYPE(rhs_dt) || rhs_dt == TYPE_BOOL)
            return 1;
        return 0;
    }
    if (IS_INT_TYPE(dst_dt)) {
        if (!IS_INT_TYPE(rhs_dt) && rhs_dt != TYPE_BOOL)
            return 0;
        if (rhs_dt == TYPE_BOOL)
            rhs_dt = TYPE_INT;
        return numeric_int_level(rhs_dt) <= numeric_int_level(dst_dt);
    }
    return 0;
}

static int visitor_binop_result_type(AST_t *node, int left_dt, int right_dt)
{
    if (left_dt == TYPE_BOOL)
        left_dt = TYPE_INT;
    if (right_dt == TYPE_BOOL)
        right_dt = TYPE_INT;
    if (node->op == DEQUALS_TOKEN || node->op == NOT_EQUALS_TOKEN ||
        node->op == LT_TOKEN || node->op == GT_TOKEN ||
        node->op == LTE_TOKEN || node->op == GTE_TOKEN)
        return TYPE_BOOL;
    if (node->op == AND_TOKEN || node->op == OR_TOKEN)
        return TYPE_BOOL;
    if (node->op == BITAND_TOKEN || node->op == BITOR_TOKEN) {
        if (!IS_INT_TYPE(left_dt) || !IS_INT_TYPE(right_dt))
            return TYPE_UNKNOWN;
        return numeric_promote_binary(left_dt, right_dt, node->op);
    }
    return numeric_promote_binary(left_dt, right_dt, node->op);
}

AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* variable = init_ast(ASSIGNEMENT_AST);
    variable->name = node->name;
    variable->datatype = node->datatype;
    variable->op = node->op;
    variable->int_value = node->int_value;

    if (node->left && node->left->type == FIELD_ACCESS_AST) {
        variable->left = visitor_visit(visitor, node->left, list, stackframe);
        if (variable->op == EQUALS_TOKEN || visitor_is_compound_assign_op(variable->op)) {
            cust_field_t *field = visitor_field_from_access(variable->left, list, stackframe);
            if (field && field->immutable)
                compile_error_ast(node, "cannot assign to immportal field '%s'", field->name);
        }
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

    if (node->parent && node->parent->type == FUNC_AST &&
        visitor->current_func && visitor->current_func->name &&
        strcmp(visitor->current_func->name, variable->name) != 0) {
        variable->datatype = node->parent->datatype;
        char buf[256];
        snprintf(buf, sizeof(buf), "%s__%s", visitor->current_func->name, variable->name);
        free(node->parent->name);
        node->parent->name = mkstr(buf);
        variable->parent = visitor_visit(visitor, node->parent, list, stackframe);
        variable->parent->id = AST_NESTED_FUNC_LITERAL;
        list_enqueue(visitor->object->children, variable->parent);
        list_enqueue(list, variable);
        return variable;
    }

    if (node->parent)
    {
        if (IS_CUST_TYPE(variable->datatype) && node->parent->type == CUST_INIT_AST &&
            node->parent->int_value < 0)
            node->parent->int_value = CUST_TYPE_ID(variable->datatype);
        variable->parent = visitor_visit(visitor, node->parent, list, stackframe);
    }

    if ((node->int_value & 1) && variable->datatype == TYPE_UNKNOWN && variable->parent) {
        int inferred = visitor_infer_datatype_from_rhs(variable->parent);
        if (inferred != TYPE_UNKNOWN)
            variable->datatype = inferred;
        else
            compile_error_ast(node, "cannot infer type for '%s'; add an explicit type", variable->name);
    }

    /* +=, -=, *=, ... require an existing variable; find its stack slot from stackframe */
    if (variable->op == PLUS_EQUALS_TOKEN || variable->op == MINUS_EQUALS_TOKEN ||
        variable->op == ASTERISK_EQUALS_TOKEN || variable->op == SLASH_EQUALS_TOKEN ||
        variable->op == MODULUS_EQUALS_TOKEN || variable->op == CARET_EQUALS_TOKEN ||
        variable->op == BITAND_EQUALS_TOKEN || variable->op == BITOR_EQUALS_TOKEN) {
        stackframe_t *owner_frame = stackframe;
        int existing = stack_lookup_for_name(stackframe, node->name, &owner_frame);
        if (existing == 0) {
            compile_error_ast(node, "variable '%s' not defined for compound assignment\n", node->name);
        }
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
                    if ((def->int_value & AST_IMMPORTAL_FLAG) && def->int_value > 0)
                        compile_error_ast(node, "cannot reassign immportal binding '%s'", node->name);
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
        if (lhs_is_arr && rhs->type == ARRAY_LITERAL_AST) {
            visitor_check_array_literal_types(rhs, variable->datatype, node);
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
        } else if (IS_NUMERIC_TYPE(variable->datatype)) {
            if (!visitor_numeric_assign_ok(variable->datatype, rhs)) {
                compile_error_ast(node, "Cannot assign %s to %s variable '%s'",
                                  rhs->type == FLOAT_AST ? "float"
                                  : rhs->type == INT_AST ? "int" : datatype_name(rhs->datatype),
                                  datatype_name(variable->datatype), variable->name);
            }
        }
    }

    if (IS_CUST_TYPE(variable->datatype)) {
        int slots = cust_type_slots(CUST_TYPE_ID(variable->datatype));
        for (int i = 0; i < slots - 1; i++)
            list_enqueue(stackframe->stack, mkstr("0"));
    } else if (IS_NUMERIC_TYPE(variable->datatype)) {
        int slots = numeric_stack_slots(variable->datatype);
        for (int i = 0; i < slots - 1; i++)
            list_enqueue(stackframe->stack, mkstr("0"));
    }

    list_enqueue(stackframe->stack, variable->name);
    variable->stack_index = stackframe->stack->size;
    variable->stackframe = stackframe;
    list_enqueue(list, variable);

    if (!tag_heap_cust_from_rent(variable, variable->parent) && variable->datatype == TYPE_ADR) {
        int immportal = variable->int_value & AST_IMMPORTAL_FLAG;
        variable->int_value = HEAP_CUST_TAG_NONE;
        if (immportal)
            variable->int_value |= AST_IMMPORTAL_FLAG;
    }
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

    if (index == 0 && node->name) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, node->name) == 0 && def->parent &&
                def->parent->type == INT_AST) {
                node->type = INT_AST;
                node->int_value = def->parent->int_value;
                node->datatype = TYPE_INT;
                free(node->name);
                node->name = NULL;
                return visit_int(visitor, node, list, stackframe);
            }
        }
        compile_error_ast(node, "Undefined variable '%s'\n", node->name);
    }

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
            if (def->multiplier >= INTERFACE_PARAM_MARK)
                node->multiplier = def->multiplier;
            if (IS_HEAP_CUST_VAR(def->datatype, def->int_value))
                node->int_value = def->int_value;
            break;
        }
    }

    /* Function parameters are not always on the module list (methods use ASSIGNEMENT_AST). */
    if (node->name) {
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
                    param->name && strcmp(param->name, node->name) == 0) {
                    if (!node->datatype || node->datatype == TYPE_UNKNOWN)
                        node->datatype = param->datatype;
                    if (IS_HEAP_CUST_VAR(param->datatype, param->int_value))
                        node->int_value = param->int_value;
                    else if (param->datatype == TYPE_ADR && param->int_value >= 0)
                        node->int_value = param->int_value;
                    if (param->multiplier >= INTERFACE_PARAM_MARK)
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
    if (expr->type == VAR_AST && expr->multiplier >= INTERFACE_PARAM_MARK)
        return -1;
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

static const char *visitor_operator_method_name(int op)
{
    switch (op) {
        case PLUS_TOKEN: return "operator_add";
        case MINUS_TOKEN: return "operator_sub";
        case ASTERISK_TOKEN: return "operator_mul";
        case SLASH_TOKEN: return "operator_div";
        case MODULUS_TOKEN: return "operator_mod";
        default: return NULL;
    }
}

static AST_t *visitor_desugar_operator_call(visitor_t *visitor, AST_t *node, AST_t *left,
                                            AST_t *right, dynamic_list_t *list,
                                            stackframe_t *stackframe)
{
    const char *mname = visitor_operator_method_name(node->op);
    if (!mname)
        return NULL;
    int cust_id = expr_cust_type_id(left, list, stackframe, visitor);
    if (cust_id < 0)
        return NULL;
    if (!cust_method_by_name(cust_id, mname))
        return NULL;

    AST_t *call = init_ast(CALL_AST);
    call->left = left;
    call->name = mkstr(mname);
    call->parent = init_ast(COMP_AST);
    list_enqueue(call->parent->children, right);
    call->source_file = node->source_file ? strdup(node->source_file) : 0;
    call->source_line = node->source_line;
    return visit_caller(visitor, call, list, stackframe);
}

static int iface_arg_is_stack_cust(AST_t *arg, dynamic_list_t *list)
{
    arg = visitor_unwrap_comp(arg);
    if (!arg)
        return 0;
    if (heap_cust_id_from_var(arg, list) >= 0)
        return 0;
    if (IS_CUST_TYPE(arg->datatype))
        return 1;
    if (arg->type == VAR_AST && arg->name) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, arg->name) == 0 &&
                IS_CUST_TYPE(def->datatype))
                return !IS_HEAP_CUST_VAR(def->datatype, def->int_value);
        }
    }
    return 0;
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

    int is_nested_literal = saved_func && saved_func != node;
    dynamic_list_t *captures = NULL;
    if (is_nested_literal)
        captures = closure_analyze_captures(node, stackframe);

    /* Register parameter names on the stack frame before visiting the body. */
    func->left = init_ast(COMP_AST);

    if (captures) {
        for (unsigned int ci = 0; ci < captures->size; ci++) {
            char *cap_name = (char *)captures->items[ci];
            AST_t *param = init_ast(ASSIGNEMENT_AST);
            param->name = mkstr(cap_name);
            param->datatype = datatype_of_var_name(list, cap_name);
            if (param->datatype == TYPE_UNKNOWN)
                compile_error_ast(node, "closure capture '%s' has unknown type\n", cap_name);
            param->multiplier = CLOSURE_CAPTURE_MARK;
            list_enqueue(func->left->children, param);
            visitor_register_func_param(visitor, param, node, new_stackframe);
        }
    }
    int skip_param_revisit = is_nested_literal && captures && captures->size > 0;
    if (captures)
        list_free_shallow(captures);

    for (int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        list_enqueue(func->left->children, child);
        visitor_register_func_param(visitor, child, node, new_stackframe);
    }

    for(int i = 0; i < node->children->size; i++)
    {
        AST_t* child = (AST_t*)node->children->items[i];
        AST_t* new_child = (skip_param_revisit && child->stack_index > 0 &&
                            child->stackframe == new_stackframe)
            ? child
            : visitor_visit(visitor, child, list, new_stackframe);
        list_enqueue(func->children, new_child);
    }

    func->parent = visitor_visit(visitor, node->parent, list, new_stackframe);
    func->stackframe = new_stackframe;

    if (is_nested_literal)
        func->id = AST_NESTED_FUNC_LITERAL;

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

    int module_call = 0;
    if (node->left && node->left->type == VAR_AST && node->left->name &&
        module_is_name(node->left->name) && node->name) {
        char *mangled = module_mangle_symbol(node->left->name, node->name);
        free(node->name);
        node->name = mangled;
        node->left = NULL;
        module_call = 1;
    }

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
        if (recv->type == VAR_AST && IS_INTERFACE_TYPE(recv->datatype))
            iface_id = INTERFACE_TYPE_ID(recv->datatype);
        else if (recv->type == VAR_AST && recv->datatype == TYPE_ADR &&
                 recv->multiplier >= INTERFACE_PARAM_MARK)
            iface_id = (int)recv->multiplier - INTERFACE_PARAM_MARK;

        cust_method_t *method = 0;
        if (iface_id >= 0) {
            interface_method_t *im = interface_method_by_name(iface_id, node->name);
            if (!im) {
                compile_error_ast(node, "method '%s' not found on interface\n", node->name);
            }
            if (im->dispatch_slot < 0) {
                compile_error_ast(node, "interface method '%s' has no implementor yet\n",
                                  node->name);
            }
            node->multiplier = CUST_CALL_VIRTUAL;
            node->int_value = im->dispatch_slot;
            node->id = 1;
            node->datatype = im->return_type;
        } else if (cust_id >= 0) {
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
        } else {
            compile_error_ast(node, "method call requires cust receiver\n");
        }
        AST_t *args = init_ast(COMP_AST);
        list_enqueue(args->children, recv);
        for (int i = 0; i < node->parent->children->size; i++) {
            AST_t *child = (AST_t *)node->parent->children->items[i];
            list_enqueue(args->children, visitor_visit(visitor, child, list, stackframe));
        }
        node->parent = args;
        if (node->multiplier != CUST_CALL_VIRTUAL)
            variable = function_lookup(visitor, list, node->name);
        if (node->multiplier != CUST_CALL_VIRTUAL && !variable) {
            compile_error_ast(node, "undefined method '%s'\n", node->name);
        }
    } else {
        for (int i = 0; i < node->parent->children->size; i++) {
            AST_t *child = (AST_t *)node->parent->children->items[i];
            list_enqueue(arguments, visitor_visit(visitor, child, list, stackframe));
        }
        variable = function_lookup(visitor, list, node->name);
        if (!variable && !module_call) {
            const char *mod = module_name_for_source(node->source_file);
            if (mod) {
                char *mangled = module_mangle_symbol(mod, node->name);
                variable = function_lookup(visitor, list, mangled);
                if (variable) {
                    free(node->name);
                    node->name = mangled;
                } else {
                    free(mangled);
                }
            }
            if (!variable) {
                for (unsigned int mi = 0; mi < module_registered_count(); mi++) {
                    const char *qmod = module_registered_name(mi);
                    if (!qmod)
                        continue;
                    char *mangled = module_mangle_symbol(qmod, node->name);
                    if (function_lookup(visitor, list, mangled)) {
                        free(mangled);
                        compile_error_ast(node,
                            "function '%s' is in module '%s'; call %s.%s(...)\n",
                            node->name, qmod, qmod, node->name);
                    }
                    free(mangled);
                }
            }
        }
        if (module_call && !variable) {
            compile_error_ast(node, "undefined module function '%s'\n", node->name);
        }
    }

    if(variable)
    {
        AST_t *func_ast = variable;
        if (variable->type == ASSIGNEMENT_AST && variable->parent &&
            variable->parent->type == FUNC_AST)
            func_ast = variable->parent;
        if (func_ast && func_ast->name) {
            free(node->name);
            node->name = mkstr(func_ast->name);
        }
        if (func_ast->id == AST_NESTED_FUNC_LITERAL)
            visitor_prepend_closure_args(visitor, node, func_ast, arguments, list, stackframe);
        check_arguments(node, func_ast);
        dynamic_list_t *params = (func_ast->left && func_ast->left->children)
            ? func_ast->left->children
            : func_ast->children;
        if (node->parent && node->parent->children) {
            for (unsigned int ai = 0; ai < node->parent->children->size; ai++) {
                AST_t *param = ai < params->size
                    ? (AST_t *)params->items[ai]
                    : 0;
                if (!param || param->multiplier < INTERFACE_PARAM_MARK ||
                    param->multiplier == CLOSURE_CAPTURE_MARK)
                    continue;
                int iid = (int)param->multiplier - INTERFACE_PARAM_MARK;
                AST_t *arg = visitor_unwrap_comp((AST_t *)node->parent->children->items[ai]);
                int cust_id = heap_cust_id_from_var(arg, list);
                if (cust_id < 0) {
                    cust_id = expr_cust_type_id(arg, list, stackframe, visitor);
                    if (cust_id < 0) {
                        compile_error_ast(node,
                                          "interface argument requires cust object or heap handle\n");
                    }
                    if (iface_arg_is_stack_cust(arg, list)) {
                        arg->int_value = cust_id;
                        arg->multiplier = INTERFACE_STACK_ARG_MARK;
                    }
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
    else if (strcmp(node->name, "PeekByte") == 0 || strcmp(node->name, "PeekInt") == 0 ||
             strcmp(node->name, "PeekI64") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "AdrLo") == 0 || strcmp(node->name, "AdrHi") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "moveOut") == 0 || strcmp(node->name, "PokeByte") == 0 ||
             strcmp(node->name, "PokeInt") == 0 || strcmp(node->name, "PokeI64") == 0 ||
             strcmp(node->name, "AddInt") == 0 ||
             strcmp(node->name, "Memcpy") == 0 || strcmp(node->name, "Memset") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "RentGrow") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "ArenaCreate") == 0 || strcmp(node->name, "PoolCreate") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "arenaRent") == 0 || strcmp(node->name, "poolRent") == 0)
        node->datatype = TYPE_ADR;
    else if (strcmp(node->name, "ArenaReset") == 0 || strcmp(node->name, "ArenaDestroy") == 0 ||
             strcmp(node->name, "PoolReset") == 0 || strcmp(node->name, "PoolDestroy") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "timer") == 0 || strcmp(node->name, "join") == 0)
        node->datatype = TYPE_INT;
    else if (strcmp(node->name, "ReadLine") == 0 || strcmp(node->name, "FileRead") == 0)
        node->datatype = TYPE_STR;
    else if (strcmp(node->name, "ReadChar") == 0 || strcmp(node->name, "KeyAvailable") == 0 ||
             strcmp(node->name, "PollKey") == 0 || strcmp(node->name, "FileOpen") == 0 ||
             strcmp(node->name, "FileWrite") == 0 || strcmp(node->name, "FileClose") == 0 ||
             strcmp(node->name, "WriteFile") == 0)
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
                   strcmp(node->name, "PeekI64") == 0 ||
                   strcmp(node->name, "AdrLo") == 0 || strcmp(node->name, "AdrHi") == 0) {
            borrow_check_adr_expr(visitor->bctx, arg0, list, " (heap op on moved adr)");
        } else if (strcmp(node->name, "PokeByte") == 0 || strcmp(node->name, "PokeInt") == 0 ||
                   strcmp(node->name, "PokeI64") == 0 ||
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
    visitor_enqueue_stack_slots(stackframe, 1);
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_float(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    visitor_enqueue_stack_slots(stackframe, numeric_stack_slots(TYPE_F64));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_F64;
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
    else {
        int ldt = binop->left->datatype;
        int rdt = binop->right->datatype;
        if (binop->left->type == INT_AST)
            ldt = TYPE_INT;
        if (binop->right->type == INT_AST)
            rdt = TYPE_INT;
        if (binop->left->type == FLOAT_AST)
            ldt = TYPE_F64;
        if (binop->right->type == FLOAT_AST)
            rdt = TYPE_F64;
        if (node->op == MODULUS_TOKEN &&
            (IS_FLOAT_TYPE(ldt) || IS_FLOAT_TYPE(rdt) ||
             binop->left->type == FLOAT_AST || binop->right->type == FLOAT_AST)) {
            compile_error_ast(node, "modulus requires integer operands");
        }
        int r = visitor_binop_result_type(binop, ldt, rdt);
        if (r == TYPE_UNKNOWN) {
            AST_t *desugared = visitor_desugar_operator_call(visitor, node, binop->left,
                                                             binop->right, list, stackframe);
            if (desugared)
                return desugared;
            compile_error_type_mismatch(node, ldt, rdt, node->op);
        }
        binop->datatype = r;
    }
    {
        int slots = (IS_NUMERIC_TYPE(binop->datatype))
            ? numeric_stack_slots(binop->datatype) : 1;
        visitor_enqueue_stack_slots(stackframe, slots);
    }
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
        if (node->left) {
            node->left = visitor_visit(visitor, node->left, list, stackframe);
            int idx;
            if (node->left->type != INT_AST && constexpr_eval_int(node->left, list, &idx)) {
                node->left->type = INT_AST;
                node->left->int_value = idx;
            }
            if (node->left->type == INT_AST && node->name)
                visitor_check_const_index(node, node->name, node->left->int_value, list);
        }
        list_enqueue(stackframe->stack, mkstr("0"));
        node->stack_index = stackframe->stack->size;
        if (IS_ARRAY_TYPE(node->parent->datatype)) {
            node->datatype = ARRAY_ELEM_TYPE(node->parent->datatype);
            node->id = type_stack_slots(node->datatype);
        } else if (node->parent->datatype == TYPE_ADR)
            node->datatype = TYPE_INT;
        return node;
    }

    /* Look up base variable index before visiting index expr (which adds to stack) */
    int base_index = stack_index_for_name(stackframe, node->name);
    if (base_index == 0 && node->name) {
        compile_error_ast(node, "Undefined variable '%s' in index access\n", node->name);
    }
    node->int_value = base_index;
    if (node->left) {
        node->left = visitor_visit(visitor, node->left, list, stackframe);
        int idx;
        if (node->left->type != INT_AST && constexpr_eval_int(node->left, list, &idx)) {
            node->left->type = INT_AST;
            node->left->int_value = idx;
        }
        if (node->left->type == INT_AST && node->name)
            visitor_check_const_index(node, node->name, node->left->int_value, list);
    }
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
            } else if (IS_ARRAY_TYPE(def->datatype)) {
                node->datatype = ARRAY_ELEM_TYPE(def->datatype);
                node->id = type_stack_slots(node->datatype);
            }
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
    if (base->type == VAR_AST && base->name) {
        int len = array_length_for_var(base->name, list);
        if (len >= 0) {
            if (start_expr->type == INT_AST)
                visitor_check_const_index(node, base->name, start_expr->int_value, list);
            if (end_expr->type == INT_AST) {
                int end = end_expr->int_value;
                if (end < 0 || end > len)
                    compile_error_ast(node, "slice end %d out of bounds for length %d", end, len);
            }
        }
    }
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

AST_t* visit_break(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    (void)visitor;
    (void)list;
    node->stackframe = stackframe;
    return node;
}

AST_t* visit_continue(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    (void)visitor;
    (void)list;
    node->stackframe = stackframe;
    return node;
}

AST_t* visit_switch(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *c = (AST_t *)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, c, list, stackframe);
    }
    node->stackframe = stackframe;
    return node;
}

AST_t* visit_case(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }
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

    if (node->name && strcmp(node->name, "len") == 0) {
        AST_t *container = node->left;
        const char *arr_name = NULL;
        int arr_dt = TYPE_UNKNOWN;
        if (container->type == VAR_AST && container->name) {
            arr_name = container->name;
            arr_dt = container->datatype;
            if (!IS_ARRAY_TYPE(arr_dt))
                arr_dt = datatype_of_var_name(list, arr_name);
        }
        if (!IS_ARRAY_TYPE(arr_dt) || !arr_name)
            compile_error_ast(node, ".len requires an array variable");
        int len = array_length_for_var(arr_name, list);
        if (len < 0)
            compile_error_ast(node, "array length of '%s' is not known at compile time", arr_name);
        node->type = INT_AST;
        node->int_value = len;
        node->string_value = NULL;
        node->datatype = TYPE_INT;
        node->left = NULL;
        free(node->name);
        node->name = NULL;
        return visit_int(visitor, node, list, stackframe);
    }

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

AST_t* visit_try(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    AST_t *assign = visitor_visit(visitor, node->left, list, stackframe);
    list_enqueue(list, assign);

    if (!IS_CUST_TYPE(assign->datatype))
        compile_error_ast(node, "try requires a Result-like cust type with bool 'ok' field");

    int cust_id = CUST_TYPE_ID(assign->datatype);
    cust_field_t *ok_field = cust_field_by_name(cust_id, "ok");
    if (!ok_field || ok_field->datatype != TYPE_BOOL)
        compile_error_ast(node, "try requires a Result-like cust type with bool 'ok' field");

    AST_t *var = init_ast(VAR_AST);
    var->name = mkstr(assign->name);

    AST_t *fa = init_ast(FIELD_ACCESS_AST);
    fa->left = var;
    fa->name = mkstr("ok");
    fa->int_value = ok_field->offset;

    AST_t *fake = init_ast(BOOL_AST);
    fake->int_value = 0;

    AST_t *cond = init_ast(BINOP_AST);
    cond->op = DEQUALS_TOKEN;
    cond->left = fa;
    cond->right = fake;

    AST_t *ifnode = init_ast(IF_AST);
    ifnode->left = cond;
    list_enqueue(ifnode->children, node->right);

    ifnode = visitor_visit(visitor, ifnode, list, stackframe);

    AST_t *comp = init_ast(COMP_AST);
    list_enqueue(comp->children, assign);
    list_enqueue(comp->children, ifnode);
    comp->stackframe = stackframe;
    if (comp->children->size > 0)
        comp->stack_index = ((AST_t *)comp->children->items[comp->children->size - 1])->stack_index;
    return comp;
}

AST_t* visit_enum(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *member = (AST_t *)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, member, list, stackframe);
    }
    return node;
}

AST_t* visit_foreach(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    if (!node->left || node->left->type != VAR_AST || !node->left->name)
        compile_error_ast(node, "foreach requires a simple array variable after 'in'");

    node->left = visitor_visit(visitor, node->left, list, stackframe);
    const char *arr_name = node->left->name;
    int arr_dt = node->left->datatype;
    if (!IS_ARRAY_TYPE(arr_dt))
        arr_dt = datatype_of_var_name(list, arr_name);
    if (!IS_ARRAY_TYPE(arr_dt))
        compile_error_ast(node, "foreach 'in' expression must be an array");

    int len = array_length_for_var(arr_name, list);
    if (len < 0)
        compile_error_ast(node, "foreach array '%s' length must be known at compile time", arr_name);

    node->int_value = len;
    node->id = node->left->stack_index;

    list_enqueue(stackframe->stack, node->name);
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;

    AST_t *idx_decl = init_ast(ASSIGNEMENT_AST);
    idx_decl->name = node->name;
    idx_decl->datatype = node->datatype ? node->datatype : TYPE_INT;
    idx_decl->stack_index = node->stack_index;
    idx_decl->stackframe = stackframe;
    idx_decl->int_value = 1;
    list_enqueue(list, idx_decl);

    borrow_block_enter(visitor->bctx);
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }
    borrow_block_leave(visitor->bctx);

    return node;
}


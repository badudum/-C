#include "include/visitor.h"
#include "include/token.h"
#include "include/cust.h"
#include "include/types.h"
#include <stdio.h>
#include <string.h>

typedef struct {
    char *name;
    int moved;
} adr_state_t;

static AST_t* visit_dupe(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

static void borrow_push_frame(visitor_t *visitor)
{
    list_enqueue(visitor->borrow_stack, visitor->borrow);
    visitor->borrow = init_list(sizeof(adr_state_t));
}

static void borrow_pop_frame(visitor_t *visitor)
{
    if (visitor->borrow_stack->size == 0)
        return;
    visitor->borrow = (dynamic_list_t*)visitor->borrow_stack->items[visitor->borrow_stack->size - 1];
    visitor->borrow_stack->size--;
}

static adr_state_t* borrow_find(visitor_t *visitor, const char *name)
{
    if (!visitor->borrow || !name)
        return 0;
    for (int i = (int)visitor->borrow->size - 1; i >= 0; i--) {
        adr_state_t *st = (adr_state_t*)visitor->borrow->items[i];
        if (st->name && strcmp(st->name, name) == 0)
            return st;
    }
    return 0;
}

static void borrow_declare(visitor_t *visitor, const char *name)
{
    adr_state_t *st = calloc(1, sizeof(adr_state_t));
    st->name = strdup(name);
    st->moved = 0;
    list_enqueue(visitor->borrow, st);
}

static void borrow_mark_moved(visitor_t *visitor, const char *name)
{
    adr_state_t *st = borrow_find(visitor, name);
    if (st)
        st->moved = 1;
}

static void borrow_check_valid(visitor_t *visitor, const char *name, const char *context)
{
    adr_state_t *st = borrow_find(visitor, name);
    if (st && st->moved) {
        fprintf(stderr, "Borrow error: use of moved adr '%s'%s\n", name, context ? context : "");
        exit(1);
    }
}

static int stack_index_for_name(stackframe_t *stackframe, const char *name)
{
    int index = 0;
    for (int i = (int)stackframe->stack->size - 1; i >= 0; i--) {
        char *var_name = (char*)stackframe->stack->items[i];
        if (var_name && strcmp(var_name, name) == 0)
            return i + 1;
    }
    return index;
}

static void borrow_check_adr_expr(visitor_t *visitor, AST_t *expr, const char *context)
{
    if (!expr)
        return;
    if (expr->type == VAR_AST && expr->datatype == TYPE_ADR)
        borrow_check_valid(visitor, expr->name, context);
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
            return def->datatype == TYPE_ADR;
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
    visitor->borrow = init_list(sizeof(adr_state_t));
    visitor->borrow_stack = init_list(sizeof(dynamic_list_t*));
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
            printf("Unknown node type: %d\n", node->type);
            exit(1);
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
    if (compound->children->size == 1) {
        compound->stack_index = ((AST_t*)compound->children->items[0])->stack_index;
    }
    return compound;
}

// this is the visitor we use when we have an assignent going on like a = 5;. This is pretty much copying everything over
static const char* datatype_name(int dt)
{
    if (IS_ARRAY_TYPE(dt)) return "Array";
    if (IS_CUST_TYPE(dt)) {
        cust_type_t *t = cust_get(CUST_TYPE_ID(dt));
        return t && t->name ? t->name : "cust";
    }
    switch(dt) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        case TYPE_ADR: return "adr";
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
            fprintf(stderr, "Error: variable '%s' not defined for += or -=\n", node->name);
            exit(1);
        }
        return variable;
    }

    /* Type mismatch checks (only for unambiguous literal/array mismatches) */
    if (variable->parent && variable->datatype != TYPE_UNKNOWN) {
        AST_t* rhs = variable->parent;
        int lhs_is_arr = IS_ARRAY_TYPE(variable->datatype);

        if (rhs->type == ARRAY_LITERAL_AST && !lhs_is_arr) {
            fprintf(stderr, "Error: Cannot assign array literal to %s variable '%s'\n",
                    datatype_name(variable->datatype), variable->name);
            exit(1);
        }
        if (lhs_is_arr && (rhs->type == INT_AST || rhs->type == STRING_AST)) {
            fprintf(stderr, "Error: Cannot assign %s to Array variable '%s'\n",
                    rhs->type == INT_AST ? "int" : "str", variable->name);
            exit(1);
        }
        if (variable->datatype == TYPE_INT && rhs->type == STRING_AST) {
            fprintf(stderr, "Error: Cannot assign str to int variable '%s'\n", variable->name);
            exit(1);
        }
        if (variable->datatype == TYPE_STR && rhs->type == INT_AST) {
            fprintf(stderr, "Error: Cannot assign int to str variable '%s'\n", variable->name);
            exit(1);
        }
        if (variable->datatype == TYPE_ADR && rhs->type == STRING_AST) {
            fprintf(stderr, "Error: Cannot assign str to adr variable '%s'\n", variable->name);
            exit(1);
        }
        if (variable->datatype == TYPE_ADR && rhs->type == INT_AST) {
            fprintf(stderr, "Error: Cannot assign int to adr variable '%s'\n", variable->name);
            exit(1);
        }
        if (variable->datatype == TYPE_STR && rhs->type == VAR_AST && rhs->datatype == TYPE_ADR) {
            fprintf(stderr, "Error: Cannot assign adr to str variable '%s'\n", variable->name);
            exit(1);
        }
        if (IS_CUST_TYPE(variable->datatype) && rhs->type == CUST_INIT_AST) {
            if (rhs->int_value != CUST_TYPE_ID(variable->datatype)) {
                fprintf(stderr, "Error: cust initializer type mismatch for '%s'\n", variable->name);
                exit(1);
            }
        } else if (IS_CUST_TYPE(variable->datatype) && rhs->type == VAR_AST &&
                   IS_CUST_TYPE(rhs->datatype) &&
                   rhs->datatype != variable->datatype) {
            fprintf(stderr, "Error: Cannot assign mismatched cust types for '%s'\n", variable->name);
            exit(1);
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

    if (variable->datatype == TYPE_ADR) {
        borrow_declare(visitor, variable->name);
        if (variable->parent && variable->parent->type == VAR_AST &&
            is_adr_var(variable->parent, list)) {
            borrow_check_valid(visitor, variable->parent->name, " (moved on assign)");
            borrow_mark_moved(visitor, variable->parent->name);
        }
    }

    return variable;
}

// This is the visitor we use when we have a variable
AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    int index = stack_index_for_name(stackframe, node->name);

    if (index == 0) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->name);
        exit(1);
    }

    node->stack_index = index;
    node->stackframe = stackframe;
    /* Latest matching declaration wins (inner scopes after outer on list) */
    for (int j = (int)list->size - 1; j >= 0; j--) {
        AST_t* def = (AST_t*)list->items[j];
        if (def->type == ASSIGNEMENT_AST && def->name && node->name &&
            strcmp(def->name, node->name) == 0) {
            node->datatype = def->datatype;
            break;
        }
    }

    /* Function parameters are VAR_AST in func->children, not on the module list. */
    if (!node->datatype && node->name && stackframe && visitor->object) {
        for (unsigned int fi = 0; fi < visitor->object->children->size; fi++) {
            AST_t *fn = (AST_t *)visitor->object->children->items[fi];
            if (fn->type != FUNC_AST || fn->stackframe != stackframe)
                continue;
            for (unsigned int pi = 0; pi < fn->children->size; pi++) {
                AST_t *param = (AST_t *)fn->children->items[pi];
                if (param->type == VAR_AST && param->name && node->name &&
                    strcmp(param->name, node->name) == 0 && param->datatype) {
                    node->datatype = param->datatype;
                    break;
                }
            }
            break;
        }
    }

    if (node->datatype == TYPE_ADR)
        borrow_check_valid(visitor, node->name, "");

    return node;
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

    borrow_push_frame(visitor);

    /* Register parameter names on the stack frame before visiting the body. */
    for (int i = 0; i < node->children->size; i++) {
        AST_t *child = (AST_t *)node->children->items[i];
        if (child->type == VAR_AST && child->name) {
            list_enqueue(new_stackframe->stack, child->name);
            child->stack_index = (int)new_stackframe->stack->size;
            child->stackframe = new_stackframe;
            if (child->datatype == TYPE_ADR)
                borrow_declare(visitor, child->name);
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

    borrow_pop_frame(visitor);

    return func;
}


// This is the part of the visitor when we want to handle a function call
AST_t* visit_caller(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    // printf("Visiting caller with token : %s \n", node->token);
    AST_t* variable = variable_lookup(visitor->object->children, node->name);
    
    dynamic_list_t * arguments = init_list(sizeof(struct AST_S));

    for(int i = 0; i < node->parent->children->size; i++)
    {
        AST_t* child = (AST_t*)node->parent->children->items[i];
        AST_t* new_child = visitor_visit(visitor, child, list, stackframe);
        list_enqueue(arguments, new_child);
    }

    if(variable)
    {
        check_arguments(node, variable);
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

    if (arguments->size > 0) {
        AST_t *arg0 = (AST_t*)arguments->items[0];
        if (strcmp(node->name, "rent") == 0 && arguments->size == 2)
            node->name = mkstr("rentMul");
        if (strcmp(node->name, "moveOut") == 0) {
            if (arg0->datatype != TYPE_ADR) {
                fprintf(stderr, "Borrow error: moveOut requires adr\n");
                exit(1);
            }
            if (arg0->type == VAR_AST) {
                borrow_check_valid(visitor, arg0->name, " (already moved)");
                borrow_mark_moved(visitor, arg0->name);
            }
        } else if (strcmp(node->name, "PeekByte") == 0 || strcmp(node->name, "PeekInt") == 0 ||
                   strcmp(node->name, "PokeByte") == 0 || strcmp(node->name, "PokeInt") == 0 ||
                   strcmp(node->name, "AddInt") == 0 || strcmp(node->name, "AdrLo") == 0 ||
                   strcmp(node->name, "AdrHi") == 0 || strcmp(node->name, "Memset") == 0) {
            borrow_check_adr_expr(visitor, arg0, " (heap op on moved adr)");
        } else if (strcmp(node->name, "Memcpy") == 0) {
            borrow_check_adr_expr(visitor, arg0, " (memcpy dst on moved adr)");
            if (arguments->size > 1) {
                AST_t *arg1 = (AST_t*)arguments->items[1];
                borrow_check_adr_expr(visitor, arg1, " (memcpy src on moved adr)");
            }
        } else if (strcmp(node->name, "RentGrow") == 0) {
            if (arg0->datatype != TYPE_ADR) {
                fprintf(stderr, "Borrow error: RentGrow requires adr\n");
                exit(1);
            }
            if (arg0->type == VAR_AST) {
                borrow_check_valid(visitor, arg0->name, " (already moved)");
                borrow_mark_moved(visitor, arg0->name);
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
    return node;
}

AST_t* visit_type_size(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->int_value <= 0) {
        fprintf(stderr, "Error: sizeof unsupported or zero-size type\n");
        exit(1);
    }
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
    if (return_node->parent && return_node->parent->type == VAR_AST &&
        return_node->parent->datatype == TYPE_ADR)
        borrow_check_valid(visitor, return_node->parent->name, " (return of moved adr)");
    return return_node;
}

AST_t* visit_access(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    /* Look up base variable index before visiting index expr (which adds to stack) */
    int base_index = stack_index_for_name(stackframe, node->name);
    if (base_index == 0 && node->name) {
        fprintf(stderr, "Error: Undefined variable '%s' in index access\n", node->name);
        exit(1);
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
                borrow_check_valid(visitor, node->name, " (index on moved adr)");
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
        fprintf(stderr, "Error: Undefined variable '%s' in slice\n", base->name);
        exit(1);
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
    if (node->left && node->left->type == VAR_AST && node->left->datatype == TYPE_ADR)
        borrow_check_adr_expr(visitor, node->left, " (passed to dupe)");
    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    node->stackframe = stackframe;
    node->datatype = TYPE_INT;
    return node;
}

AST_t* visit_if(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    if (node->left)
        node->left = visitor_visit(visitor, node->left, list, stackframe);

    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t* child = (AST_t*)node->children->items[i];
        node->children->items[i] = visitor_visit(visitor, child, list, stackframe);
    }

    if (node->right)
        node->right = visitor_visit(visitor, node->right, list, stackframe);

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
    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t* child = (AST_t*)node->children->items[i];
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
    (void)visitor;
    (void)list;
    (void)stackframe;
    if (cust_lookup_by_name(node->name) < 0)
        node->int_value = cust_register_from_ast(node->name, node->children);
    else
        node->int_value = cust_lookup_by_name(node->name);
    node->datatype = MAKE_CUST_TYPE(node->int_value);
    return node;
}

AST_t* visit_cust_init(visitor_t *visitor, AST_t *node, dynamic_list_t *list, stackframe_t *stackframe)
{
    (void)visitor;
    if (node->int_value < 0) {
        fprintf(stderr, "Error: cust initializer requires a typed variable or Type{...} syntax\n");
        exit(1);
    }
    cust_type_t *type = cust_get(node->int_value);
    if (!type) {
        fprintf(stderr, "Error: unknown cust type in initializer\n");
        exit(1);
    }

    for (unsigned int i = 0; i < node->children->size; i++) {
        AST_t *entry = (AST_t *)node->children->items[i];
        cust_field_t *field = cust_field_by_name(node->int_value, entry->name);
        if (!field) {
            fprintf(stderr, "Error: unknown field '%s' in cust '%s' initializer\n",
                    entry->name, type->name);
            exit(1);
        }
        entry->parent = visitor_visit(visitor, entry->parent, list, stackframe);
        entry->int_value = field->offset;
        entry->datatype = field->datatype;
    }

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
    AST_t *container = node->left;

    if (container->type == VAR_AST) {
        int dt = container->datatype;
        if (!IS_CUST_TYPE(dt))
            dt = datatype_of_var_name(list, container->name);
        if (!IS_CUST_TYPE(dt)) {
            fprintf(stderr, "Error: field access on non-cust variable '%s'\n",
                    container->name ? container->name : "?");
            exit(1);
        }
        cust_id = CUST_TYPE_ID(dt);
        container->stack_index = stack_index_for_name(stackframe, container->name);
        if (container->stack_index == 0) {
            fprintf(stderr, "Error: undefined variable '%s' in field access\n", container->name);
            exit(1);
        }
    } else if (container->type == FIELD_ACCESS_AST) {
        if (!IS_CUST_TYPE(container->datatype)) {
            fprintf(stderr, "Error: nested field access requires cust field type\n");
            exit(1);
        }
        cust_id = CUST_TYPE_ID(container->datatype);
        parent_offset = container->int_value;
    } else {
        fprintf(stderr, "Error: invalid base for field access\n");
        exit(1);
    }

    cust_field_t *field = cust_field_by_name(cust_id, node->name);
    if (!field) {
        fprintf(stderr, "Error: field '%s' not found in cust type\n", node->name);
        exit(1);
    }

    node->datatype = field->datatype;
    node->int_value = parent_offset + field->offset;
    AST_t *root = field_access_root(node);
    if (root && root->type == VAR_AST)
        node->id = stack_index_for_name(stackframe, root->name);
    node->stackframe = stackframe;

    list_enqueue(stackframe->stack, mkstr("0"));
    node->stack_index = stackframe->stack->size;
    return node;
}


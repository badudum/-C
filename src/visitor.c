#include "include/visitor.h"
#include "include/token.h"
#include <stdio.h>
#include <string.h>


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
    return visitor;
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
    switch(dt) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        default: return "unknown";
    }
}

AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* variable = init_ast(ASSIGNEMENT_AST);
    variable->name = node->name;
    variable->datatype = node->datatype;
    variable->op = node->op;

    if (node->parent)
    {
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
    }

    list_enqueue(stackframe->stack, variable->name);
    variable->stack_index = stackframe->stack->size;
    variable->stackframe = stackframe;


    return variable;
}

// This is the visitor we use when we have a variable
AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    list_enqueue(stackframe->stack, 0);
    
    int index = 0;

    for (int i = 0; i < stackframe->stack->size; i++) {
        char* var_name = (char*)stackframe->stack->items[i];
        if (var_name && strcmp(var_name, node->name) == 0) {
            index = i + 1;
            break;
        }
    }

    if (index == 0) {
        fprintf(stderr, "Error: Undefined variable '%s'\n", node->name);
        exit(1);
    }

    node->stack_index = index;
    node->stackframe = stackframe;

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

    for(int i = 0; i < node->children->size; i++)
    {
        AST_t* child = (AST_t*)node->children->items[i];
        AST_t* new_child = visitor_visit(visitor, child, list, new_stackframe);
        list_enqueue(func->children, new_child);
    }

    for(int i = 0; i < func->children->size; i++)
    {
        list_enqueue(list, func->children->items[i]);
    }

    func->parent = visitor_visit(visitor, node->parent, list, new_stackframe);
    func->stackframe = new_stackframe;

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
    return return_node;
}

AST_t* visit_access(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    /* Look up base variable index before visiting index expr (which adds to stack) */
    int base_index = 0;
    for (int i = 0; i < stackframe->stack->size; i++) {
        char* var_name = (char*)stackframe->stack->items[i];
        if (var_name && node->name && strcmp(var_name, node->name) == 0) {
            base_index = i + 1;
            break;
        }
    }
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


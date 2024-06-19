#include "include/visitor.h"
#include <stdio.h>
#include <string.h>


 AST_t* variable_lookup(dynamic_list_t* list, char* name)
{
    for (int i = 0; i < list->size; i++)
    {
        AST_t* children = (AST_t*)list->items[i];
        if((children->type != VAR_AST && children->type != FUNC_AST) || !children->name)
        {
            continue;
        }
        if (strcmp(children->name, name) == 0)
        {
            return children;
        }
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
        case COMP_AST: return visit_compound(visitor, node, list, node->stackframe); break;
        case ASSIGNEMENT_AST: return visit_assignment(visitor, node, list, node->stackframe);break;
        case VAR_AST: return visit_var(visitor, node, list, node->stackframe);break;
        case FUNC_AST: return visit_func(visitor, node, list, node->stackframe);break;
        case CALL_AST: return visit_caller(visitor, node, list, node->stackframe);break;
        case INT_AST: return visit_int(visitor, node, list, node->stackframe);break;
        case STRING_AST: return visit_str(visitor, node, list, node->stackframe);break;
        case BINOP_AST: return visit_binop(visitor, node, list, node->stackframe);break;
        case RETURN_AST: return visit_return(visitor, node, list, node->stackframe);break;
        case ACCESS_AST: return visit_access(visitor, node, list, node->stackframe);break;
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
}

// this is the visitor we use when we have an assignent going on like a = 5;. This is pretty much copying everything over
AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* variable = init_ast(VAR_AST);
    variable->name = node->name;
    variable->datatype = node->datatype;

    if (node->parent)
    {
        variable->parent = visitor_visit(visitor, node->parent, list, stackframe);
    }

    variable->stack_index = stackframe->stack->size;
    variable->stackframe = stackframe;

    list_enqueue(stackframe->stack, variable);

    return variable;
}

// This is the visitor we use when we have a variable
AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    list_enqueue(stackframe->stack, 0);
    
    int index = 0;

    for (int i = 0; i < list->size; i++)
    {
        AST_t * child = (AST_t*)list->items[i];

        if (!child->name)
        {
            continue;
        }

        if(strcmp(child->name, node->name) == 0)
        {
            index = i +1 ;
            break;
        }
    }

    node->stack_index = index ? (index + 1) : list_index_deep(stackframe->stack, node);
    node->stackframe = stackframe;

    return node;
}

// This is the visitor we use when we have a function definition
AST_t* visit_func(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t* func = init_ast(FUNC_AST);
    func->name = node->name;
    func->datatype = node->datatype;
    func->children =  init_list(sizeof(struct AST_S));

    stackframe_t * new_stackframe = init_stackframe();
    list_enqueue(stackframe->stack, 0);

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
    AST_t* variable = variable_lookup(visitor->object->children, node->name);
    
    dynamic_list_t * arguments = init_list(sizeof(struct AST_S));

    for(int i = 0; i < node->children->size; i++)
    {
        AST_t* child = (AST_t*)node->children->items[i];
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
    node->stack_index = -(stackframe->stack->size);
    node->stackframe = stackframe;
    list_enqueue(stackframe->stack, mkstr("0"));
    return node;
}

AST_t* visit_str(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    dynamic_list_t * string = str_to_hex_list(node->string_value);

    list_enqueue(stackframe->stack, 0);
    node->stack_index = -(stackframe->stack->size + string->size);

    return node;
}

// This is the visitor we use when we have a binary operation
AST_t* visit_binop(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe)
{
    AST_t * binop = init_ast(BINOP_AST);
    binop->left = visitor_visit(visitor, node->left, list, stackframe);
    binop->right = visitor_visit(visitor, node->right, list, stackframe);
    binop->op = node->op;
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
    list_enqueue(stackframe->stack, 0);
    node->stack_index = list_index_deep(stackframe->stack, node);
    return node;
}


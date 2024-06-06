#ifndef AST_H
#define AST_H
#include <stdlib.h>
#include "types.h"
#include "list.h"
#include "stackframe.h"

// struct for the abstract syntax tree
typedef struct AST_S
{
    enum 
    {   
        COMP_AST,
        FUNC_AST,
        CALL_AST,
        ASSIGNEMENT_AST = 99,
        DEF_TYPE_AST,
        VAR_AST,
        RETURN_AST,
        ACCESS_AST,
        INT_AST,
        STRING_AST,
        BINOP_AST,
        NOOP_AST,
    }type;

    datatype datatype;

    dynamic_list_t * children;

    char * string_value;
    char * name;

    struct AST_S * left;
    struct AST_S * right;
    struct AST_S * parent;

    int op;
    int int_value;
    int id;
    int stack_index;
    int multiplier;

    struct AST_S * (*fptr)(struct VISITOR_S * visitor, struct AST_S * node, dynamic_list_t * list);
    stackframe_t * stackframe;  
} AST_t;

AST_t *  init_ast(int type);

#endif 
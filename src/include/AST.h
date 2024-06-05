#ifndef AST_H
#define AST_H
#include <stdlib.h>

// struct for the abstract syntax tree
typedef struct AST_S
{
    enum 
    {   
        VARIABLE_DEF_AST,
        VARIABLE_AST,
        FUNCTION_CALL_AST,
        STRING_AST,
        COMPOUND_AST //this is a list of multiple statements
    }type;

    // AST variable definitions
    char * var_def_name;
    struct AST_S* var_def_val;

    // AST variable 
    char * var_name;

    // AST function call
    char * func_name;
    struct AST_S ** func_arg;
    size_t func_arg_size;

    // AST string

    char * str_val;

    // AST compound

    struct AST_S ** compound_val;
    size_t compound_size;

} AST_t;

AST_t *  init_ast(int type);

#endif 
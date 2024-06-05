#include "include/AST.h"


AST_t * init_ast(int type)
{
    AST_t * ast = calloc(1, sizeof(struct AST_S));
    ast->type = type;

    ast->var_def_name = (void *) 0;
    ast->var_def_val = (void *) 0;

    ast->var_name = (void* ) 0;

    ast->func_name = (void*)0;
    ast->func_arg = (void*)0;
    ast->func_arg_size = 0;

    ast->str_val = (void*)0;

    ast->compound_val = (void*)0;
    ast->compound_size = 0;
    return ast;
}
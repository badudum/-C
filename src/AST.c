#include "include/AST.h"


AST_t * init_ast(int type)
{
    AST_t * ast = calloc(1, sizeof(struct AST_S));
    ast->type = type;

    if (type == COMP_AST)
    {
        ast->children = init_list(sizeof(struct AST_S*));
    }

    ast->stackframe = (void*)0;
    ast->multiplier = 1;
    ast->datatype = TYPE_UNKNOWN;
    return ast;
}
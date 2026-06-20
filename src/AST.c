#include "include/AST.h"
#include "include/types.h"
#include "include/parser.h"
#include "include/token.h"
#include <string.h>
#include <stdlib.h>


AST_t * init_ast(int type)
{
    AST_t * ast = calloc(1, sizeof(struct AST_S));
    ast->type = type;

    if (type == COMP_AST || type == SLICE_AST || type == ARRAY_LITERAL_AST || type == IF_AST || type == LOOP_UNTIL_AST || type == FOR_CLAUSE_AST || type == CUST_DEF_AST || type == CUST_INIT_AST)
    {
        ast->children = init_list(sizeof(struct AST_S*));
    }

    ast->stackframe = (void*)0;
    ast->multiplier = 0;
    ast->int_value = 0;
    ast->datatype = TYPE_UNKNOWN;
    ast->source_line = 0;
    ast->source_file = NULL;
    return ast;
}

void ast_set_loc_from_token(AST_t *ast, token_t *token)
{
    if (!ast || !token)
        return;
    ast->source_line = (int)token->line;
    if (ast->source_file) {
        free(ast->source_file);
        ast->source_file = NULL;
    }
    if (token->source_file)
        ast->source_file = strdup(token->source_file);
}

void ast_set_loc_from_parser(AST_t *ast, parser_t *parser)
{
    if (!parser)
        return;
    ast_set_loc_from_token(ast, parser->token);
}

AST_t *parser_make_ast(parser_t *parser, int type)
{
    AST_t *ast = init_ast(type);
    ast_set_loc_from_parser(ast, parser);
    return ast;
}

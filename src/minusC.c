#include "include/minusC.h"
#include <stdio.h>

void compile(char * src)
{

    lexer_t* lexer = init_lexer(src);
    token_t * token = (void*)0;
    while ((token = lexer_get_next_token(lexer)) != (void*)0)
    {
        printf("TOKEN(%d, %s)\n", token->type, token->value);
    }
    parser_t* parser = init_parser(lexer);
    AST_t * root = parse(parser);

    printf("%d\n", root->children->size);
    visitor_t*  visitor = init_visitor();
    AST_t * optimized_root = visitor_visit(visitor, root, init_list(sizeof(struct AST_S*)), init_stackframe());

    
}

void minusCompile_file(const char* filename)
{
    char * src = read_file(filename);

    compile(src);
    free(src);
}




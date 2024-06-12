#include "include/minusC.h"
#include <stdio.h>

void compile(char * src)
{

    lexer_t* lexer = init_lexer(src);
    // while ((token = lexer_get_next_token(lexer)) != (void*)0)
    // {
    //     printf("TOKEN(%d, %s)\n", token->type, token->value);
    // }
    parser_t* parser = init_parser(lexer);
    AST_t * root = parse(parser);

    
    visitor_t*  visitor = init_visitor();
    AST_t * optimized_root = visitor_visit(visitor, root, init_list(sizeof(struct AST_S*)), init_stackframe());

    char * ass = assemble(root, init_list(sizeof(struct AST_S*)));
    
}

void minusCompile_file(const char* filename)
{
    char * src = read_file(filename);

    compile(src);
    free(src);
}




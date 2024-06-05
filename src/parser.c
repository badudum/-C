#include "include/parser.h"
#include <stdio.h>
#include "include/AST.h"
#include "include/token.h"


parser_t * init_parser(lexer_t * lexer)
{
    parser_t * parser = calloc(1, sizeof(struct PARSER_S));
    parser->lexer = lexer;
    parser->token = lexer_get_next_token(lexer);
 
    return parser;
}

void parser_next(parser_t * parser, int token_type)
{
    if(parser->token->type == token_type)
    {
        parser->token = lexer_get_next_token(parser->lexer);
    }
    else
    {
        printf(
            "Incorrect token '%s', with type %d",
            parser->token->value,
            parser->token->type
        );
    }
}

AST_t * parser_parse(parser_t* parser)
{
    return parse_statements();
}

AST_t * parse_statement(parser_t* parser)
{
    switch (parser->token->type)
    {
        case ID_TOKEN : return parse_id(parser);
    }

}

AST_t * parse_statements(parser_t* parser)
{

    // in the beginning we set up everything first, where we initilaize the AST, and get the first statement, putting it into the compound value.
    AST_t * compound = init_ast(COMPOUND_AST); 
    AST_t * ast_statement = parse_statement(parser); 
    
    compound->compound_val = calloc(1, sizeof(struct AST_S)); 

    compound->compound_val[0] = ast_statement;

    // while we have semicolons, we will keep parsing statemnts while we increase the list size and reallocating more memory
    while(parser->token->type == SEMI_TOKEN)
    {
        parser_next(parser, SEMI_TOKEN);

        AST_t * ast_statement = parse_statement(parser);
        compound->compound_size += 1;
        compound->compound_val = realloc(compound->compound_val, compound->compound_size * sizeof(struct AST_S));
        compound->compound_val[compound->compound_size-1] = ast_statement;
    }

    return compound;
}

AST_t * parse_exp(parser_t * parser)
{

}

AST_t * parse_factor(parser_t * parser)
{

}

AST_t * parse_term(parser_t * parser)
{

}

AST_t * parse_func_call(parser_t* parser)
{

}

AST_t * parse_var(parser_t* parser)
{

}

AST_t * parse_str(parser_t* parser)
{

}

AST_t * parse_id(parser_t* parser)
{
    if (strcmp(parser->token->value, "int") == 0 )
    {
        return parse_variable_definition(parser);
    }
    else if(strcmp(parser->token->value, "var"))
    else
    {
        return parse_var(parser);
    }
}

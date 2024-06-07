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

token_t * parser_next(parser_t * parser, int token_type)
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

AST_t * parse(parser_t* parser)
{
    return parse_compound(parser);
}

// THIS IS WHERE THE IMPORTANT STUFF HAPPENS (SYNTACTIC STUFF). MODIFY THIS AND THE LEXER TO ACHIEVE DIFFERENT STYLES OF CODE IG.
AST_t * parse_id(parser_t * parser)
{
    char * value  = calloc(strlen(parser->token->value) + 1, sizeof(char));
    strcpy(value, parser->token->value);
    parser_next(parser, ID_TOKEN);

    AST_t * ast = init_ast(VAR_AST);
    ast->name = value;

}

AST_t * parse_factor(parser_t * parser)
{

}

AST_t * parse_term(parser_t *parser);

AST_t * parse_expression(parser_t* parser);

AST_t * parse_statement(parser_t * parser);

AST_t * parse_list(parser_t * parser);

AST_t * parse_string(parser_t * parser)
{

}

AST_t * parse_compound(parser_t * parser)
{

}



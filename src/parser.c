#include "include/parser.h"
#include <stdio.h>
#include "include/AST.h"
#include "include/token.h"
#include <stdbool.h>


AST_t * parse_int(parser_t * parser)
{
    int value = atoi(parser->token->value);
    parser_next(parser, INT_TOKEN);

    AST_t * ast = init_ast(INT_AST);
    ast->int_value = value;

    return ast;
}



parser_t * init_parser(lexer_t * lexer)
{
    parser_t * parser = calloc(1, sizeof(struct PARSER_S));
    parser->lexer = lexer;
    parser->token = lexer_get_next_token(lexer);
 
    return parser;
}

token_t * parser_next(parser_t * parser, int token_type)
{
    // printf("[Next]Current token : %s\n", parser->token->value);
    if(parser->token->type == token_type)
    {
        parser->token = lexer_get_next_token(parser->lexer);
    }
    else
    {
        printf(
            "[Parser Next] Incorrect token '%s', with type %d, expected token type %d\n",
            parser->token->value,
            parser->token->type,
            token_type
        );
    }
}

AST_t * parse(parser_t* parser)
{
    return parse_compound(parser);
}


// =================================================================================================
// THIS IS WHERE THE IMPORTANT STUFF HAPPENS (SYNTACTIC STUFF).
// MODIFY BELOW HERE AND THE LEXER TO ACHIEVE DIFFERENT STYLES OF CODE IG.
// =================================================================================================



AST_t * parse_id(parser_t * parser) // this part mainly handles vairable declaration
{
    // printf("[ID] Current Token : %s\n", parser->token->value);
    char * value  = calloc(strlen(parser->token->value) + 1, sizeof(char));
    strcpy(value, parser->token->value);
    parser_next(parser, ID_TOKEN);

    AST_t * ast = init_ast(VAR_AST);
    ast->name = value;


    // this is for varaible assignemnt
    if(parser->token->type == RBRACE_TOKEN)
    {
        parser_next(parser, RBRACE_TOKEN);
        while(parser->token->type == ID_TOKEN)
        {
            ast->datatype = type_to_type(parser->token->value);
            parser_next(parser, ID_TOKEN);

            if (parser->token->type == LT_TOKEN)// for things like Array<int>
            {
                parser_next(parser, LT_TOKEN);
                ast->datatype += type_to_type(parser->token->value);
                parser_next(parser, ID_TOKEN);
                parser_next(parser, GT_TOKEN);
            }
        }
    }

    else 
    {
        if (parser->token->type == LPAREN_TOKEN) // this is for function calls
        {
            ast->type = CALL_AST;
            ast->parent = parse_list(parser);
        }
        else if (parser->token->type == LSQUAREBRKT_TOKEN) // this is for array access
        {
            parser_next(parser, LSQUAREBRKT_TOKEN);
            ast->type = ACCESS_AST;
            ast->int_value = atoi(parser->token->value);
            parser_next(parser, INT_TOKEN);
            parser_next(parser, RSQUAREBRKT_TOKEN);
        }
    }

    if (parser->token->type == EQUALS_TOKEN)
    {
        parser_next(parser, EQUALS_TOKEN);
        ast->type = ASSIGNEMENT_AST;
        ast->name = value;

        ast->parent = parse_expression(parser);

        if (ast->parent->type != CALL_AST)
        {
            ast->parent->name = mkstr(ast->name);
        }
    }

    return ast;


}

//This is the function used to parse factors
AST_t * parse_factor(parser_t * parser)
{
    // printf("[Factor] Current Token : %s\n", parser->token->value);
    switch (parser->token->type)
    {
        case INT_TOKEN:
            return parse_int(parser);
        case ID_TOKEN:
            return parse_id(parser);
        case STRING_TOKEN:
            return parse_string(parser);
        case LPAREN_TOKEN:
            return parse_list(parser);
        case LBRACE_TOKEN:{
            parser_next(parser, parser->token->type);
            return parse_id(parser);
        }
        default:
            printf("[Parse Factor] Unexpected token %s, with value : %d\n", parser->token->value, parser->token->type);
            exit(1);
    }
}

//this is for multiplication and division
AST_t * parse_term(parser_t *parser)
{
    // printf("[Term] Current Token : %s\n", parser->token->value);
    AST_t * left = parse_factor(parser);

    while (parser->token->type == ASTERISK_TOKEN || parser->token->type == SLASH_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        left->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_factor(parser);
        return binop;
    }

    return left;
}

//this is for addition and subtraction
AST_t * parse_expression(parser_t* parser)
{
    // printf("[Expression] Current Token : %s\n", parser->token->value);
    AST_t * left = parse_term(parser); // parse the left side of the expression aka "{x} int"

    while (parser->token->type == PLUS_TOKEN || parser->token->type == MINUS_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        left->op = parser->token->type;
        binop->right= parse_expression(parser);
        parser_next(parser, parser->token->type);
        return binop;
    }
    return left;
}


//This is the function used to parse return statements
AST_t * parse_statement(parser_t * parser)
{
    // printf("[Statement] Current Token : %s\n", parser->token->value);
    AST_t * ast = init_ast(RETURN_AST);
    parser_next(parser, RETURN_TOKEN);
    ast->parent = parse_expression(parser);

    return ast;

}


//This is the function used to parse lists
AST_t * parse_list(parser_t * parser)
{
    // printf("[List] Current Token : %s\n", parser->token->value);
    bool is_bracket = parser->token->type == LSQUAREBRKT_TOKEN;
    bool is_brace = parser->token->type == LBRACE_TOKEN;

    if (!is_brace)
    {
        parser_next(parser, is_bracket ? LSQUAREBRKT_TOKEN : LPAREN_TOKEN);
    }

    AST_t * list = init_ast(COMP_AST);

    if(parser->token->type == is_bracket ? RSQUAREBRKT_TOKEN : RPAREN_TOKEN)
    {
        list_enqueue(list->children, parse_expression(parser));

        while (parser->token->type == COMMA_TOKEN)
        {
            parser_next(parser, COMMA_TOKEN);
            list_enqueue(list->children, parse_expression(parser));
        }
    }

    if(!is_brace)
    {
        parser_next(parser, is_bracket ? RSQUAREBRKT_TOKEN : RPAREN_TOKEN);
    }


    if (parser->token->type == RBRACE_TOKEN)
    {
        parser_next(parser, RBRACE_TOKEN);
        while(parser->token->type == ID_TOKEN)
        {
            list->datatype += type_to_type(parser->token->value);
            parser_next(parser, ID_TOKEN);

            if (parser->token->type == LT_TOKEN)
            {
                parser_next(parser, LT_TOKEN);
                list->datatype += type_to_type(parser->token->value);
                parser_next(parser, ID_TOKEN);
                parser_next(parser, GT_TOKEN);
            }

        }
    }

    if(parser->token->type == FUNCTION_TOKEN)
    {
        parser_next(parser, FUNCTION_TOKEN);
        list->type = FUNC_AST;
        list->parent = parse_compound(parser);

        // TODO : apprenetly we need to implement this 

        for (int i = 0; i < list->children->size; i++)
        {
            ((AST_t*)list->children->items[i])->type = ASSIGNEMENT_AST;
        }

    }

    return list;
        
}


//This is the function used to parse strings
AST_t * parse_string(parser_t * parser)
{
    char * value = mkstr(parser->token->value);
    parser_next(parser, STRING_TOKEN);

    AST_t * ast = init_ast(STRING_AST);
    ast->string_value = value;
    return ast;
}

//This is the function used to create a "compound" or pretty much a block of code with {}
AST_t * parse_compound(parser_t * parser)
{
    bool is_braced = false;


    if (parser->token->type == LBRACE_TOKEN)
    {
        is_braced = true;
        parser_next(parser, LBRACE_TOKEN);
    }

    AST_t * compound = init_ast(COMP_AST);

    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
    {
        // printf("[Compound] Current token: %s\n", parser->token->value);

        if (parser->token->type == RETURN_TOKEN)
        {
            list_enqueue(compound->children, parse_statement(parser)); //parse_statement is for returns
        }
        else 
        {
            list_enqueue(compound->children, parse_expression(parser)); // this is for all other tokens between the braces
        }
        if (parser->token->type == SEMI_TOKEN) //we skip the semi colons
        {
            parser_next(parser, SEMI_TOKEN);
        }
    }

    if(is_braced)
    {
        parser_next(parser, RBRACE_TOKEN);
    }

    return compound;

}



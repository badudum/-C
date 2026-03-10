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
        exit(1);
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
        else if (parser->token->type == LSQUAREBRKT_TOKEN)
        {
            parser_next(parser, LSQUAREBRKT_TOKEN);
            AST_t* start_expr = parse_expression(parser);
            if (parser->token->type == COLON_TOKEN)
            {
                parser_next(parser, COLON_TOKEN);
                AST_t* end_expr = parse_expression(parser);
                parser_next(parser, RSQUAREBRKT_TOKEN);
                AST_t* slice_ast = init_ast(SLICE_AST);
                list_enqueue(slice_ast->children, ast);
                list_enqueue(slice_ast->children, start_expr);
                list_enqueue(slice_ast->children, end_expr);
                return slice_ast;
            }
            else
            {
                parser_next(parser, RSQUAREBRKT_TOKEN);
                ast->type = ACCESS_AST;
                ast->left = start_expr;
            }
        }
    }

    if (parser->token->type == EQUALS_TOKEN)
    {
        parser_next(parser, EQUALS_TOKEN);
        ast->type = ASSIGNEMENT_AST;
        ast->name = value;

        ast->parent = parse_expression(parser);

        if (ast->parent->type != CALL_AST &&
            ast->parent->type != ACCESS_AST &&
            ast->parent->type != SLICE_AST)
        {
            ast->parent->name = mkstr(ast->name);
        }
    }

    return ast;


}

AST_t * parse_bool(parser_t * parser)
{
    AST_t * ast = init_ast(BOOL_AST);
    ast->int_value = (parser->token->type == REAL_TOKEN) ? 1 : 0;
    parser_next(parser, parser->token->type);
    return ast;
}

//This is the function used to parse factors
AST_t * parse_factor(parser_t * parser)
{
    switch (parser->token->type)
    {
        case INT_TOKEN:
            return parse_int(parser);
        case ID_TOKEN:
            return parse_id(parser);
        case STRING_TOKEN:
            return parse_string(parser);
        case REAL_TOKEN:
        case FAKE_TOKEN:
            return parse_bool(parser);
        case LPAREN_TOKEN:
            return parse_list(parser);
        case LSQUAREBRKT_TOKEN:{
            parser_next(parser, LSQUAREBRKT_TOKEN);
            AST_t* arr = init_ast(ARRAY_LITERAL_AST);
            if (parser->token->type != RSQUAREBRKT_TOKEN) {
                AST_t* first = parse_expression(parser);
                if (parser->token->type == SEMI_TOKEN) {
                    AST_t* val = first;
                    while (1) {
                        parser_next(parser, SEMI_TOKEN);
                        AST_t* count_expr = parse_expression(parser);
                        if (count_expr->type != INT_AST) {
                            fprintf(stderr, "Error: Array repeat count must be an integer literal\n");
                            exit(1);
                        }
                        int count = count_expr->int_value;
                        for (int i = 0; i < count; i++) {
                            AST_t* elem = init_ast(INT_AST);
                            elem->int_value = val->int_value;
                            list_enqueue(arr->children, elem);
                        }
                        if (parser->token->type == COMMA_TOKEN) {
                            parser_next(parser, COMMA_TOKEN);
                            val = parse_expression(parser);
                            if (parser->token->type != SEMI_TOKEN) {
                                fprintf(stderr, "Error: Expected ';' after value in array range syntax\n");
                                exit(1);
                            }
                        } else {
                            break;
                        }
                    }
                } else {
                    list_enqueue(arr->children, first);
                    while (parser->token->type == COMMA_TOKEN) {
                        parser_next(parser, COMMA_TOKEN);
                        list_enqueue(arr->children, parse_expression(parser));
                    }
                }
            }
            parser_next(parser, RSQUAREBRKT_TOKEN);
            arr->int_value = arr->children->size;
            return arr;
        }
        case LBRACE_TOKEN:{
            parser_next(parser, parser->token->type);
            return parse_id(parser);
        }
        default:
            printf("[Parse Factor] Unexpected token %s, with value : %d\n", parser->token->value, parser->token->type);
            exit(1);
    }
}

// unary: not, ~
AST_t * parse_unary(parser_t * parser)
{
    if (parser->token->type == NOT_TOKEN || parser->token->type == BITNOT_TOKEN) {
        AST_t * unary = init_ast(UNARY_AST);
        unary->op = parser->token->type;
        parser_next(parser, parser->token->type);
        unary->left = parse_unary(parser);
        return unary;
    }
    return parse_factor(parser);
}

// multiplication, division
AST_t * parse_term(parser_t *parser)
{
    AST_t * left = parse_unary(parser);

    while (parser->token->type == ASTERISK_TOKEN || parser->token->type == SLASH_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_unary(parser);
        left = binop;
    }

    return left;
}

// addition, subtraction
AST_t * parse_addition(parser_t* parser)
{
    AST_t * left = parse_term(parser);

    while (parser->token->type == PLUS_TOKEN || parser->token->type == MINUS_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_term(parser);
        left = binop;
    }
    return left;
}

// bitwise and
AST_t * parse_bitand(parser_t* parser)
{
    AST_t * left = parse_addition(parser);

    while (parser->token->type == BITAND_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_addition(parser);
        left = binop;
    }
    return left;
}

// bitwise or
AST_t * parse_bitor(parser_t* parser)
{
    AST_t * left = parse_bitand(parser);

    while (parser->token->type == BITOR_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_bitand(parser);
        left = binop;
    }
    return left;
}

// comparisons: ==  !=  <  >  <=  >=
AST_t * parse_comparison(parser_t* parser)
{
    AST_t * left = parse_bitor(parser);

    while (parser->token->type == DEQUALS_TOKEN ||
           parser->token->type == NOT_EQUALS_TOKEN ||
           parser->token->type == LT_TOKEN ||
           parser->token->type == GT_TOKEN ||
           parser->token->type == LTE_TOKEN ||
           parser->token->type == GTE_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_bitor(parser);
        left = binop;
    }
    return left;
}

// logical and
AST_t * parse_and(parser_t* parser)
{
    AST_t * left = parse_comparison(parser);

    while (parser->token->type == AND_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_comparison(parser);
        left = binop;
    }
    return left;
}

// logical or  (lowest precedence)
AST_t * parse_expression(parser_t* parser)
{
    AST_t * left = parse_and(parser);

    while (parser->token->type == OR_TOKEN)
    {
        AST_t * binop = init_ast(BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_and(parser);
        left = binop;
    }
    return left;
}


//This is the function used to parse return statements
AST_t * parse_statement(parser_t * parser)
{
    AST_t * ast = init_ast(RETURN_AST);
    parser_next(parser, RETURN_TOKEN);
    ast->parent = parse_expression(parser);
    return ast;
}

/*
 * if (condition) { body } else if (condition) { body } else { body };
 * IF_AST: left=condition, children=body, right=next else/else-if (IF_AST or NULL)
 */
AST_t * parse_if(parser_t * parser)
{
    parser_next(parser, IF_TOKEN);
    AST_t * node = init_ast(IF_AST);

    parser_next(parser, LPAREN_TOKEN);
    node->left = parse_expression(parser);
    parser_next(parser, RPAREN_TOKEN);

    parser_next(parser, LBRACE_TOKEN);
    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
    {
        if (parser->token->type == RETURN_TOKEN)
            list_enqueue(node->children, parse_statement(parser));
        else if (parser->token->type == IF_TOKEN)
            list_enqueue(node->children, parse_if(parser));
        else
            list_enqueue(node->children, parse_expression(parser));
        if (parser->token->type == SEMI_TOKEN)
            parser_next(parser, SEMI_TOKEN);
    }
    parser_next(parser, RBRACE_TOKEN);

    if (parser->token->type == ELSE_TOKEN)
    {
        parser_next(parser, ELSE_TOKEN);
        if (parser->token->type == IF_TOKEN)
        {
            node->right = parse_if(parser);
        }
        else
        {
            AST_t * else_node = init_ast(IF_AST);
            else_node->left = NULL;
            parser_next(parser, LBRACE_TOKEN);
            while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
            {
                if (parser->token->type == RETURN_TOKEN)
                    list_enqueue(else_node->children, parse_statement(parser));
                else if (parser->token->type == IF_TOKEN)
                    list_enqueue(else_node->children, parse_if(parser));
                else
                    list_enqueue(else_node->children, parse_expression(parser));
                if (parser->token->type == SEMI_TOKEN)
                    parser_next(parser, SEMI_TOKEN);
            }
            parser_next(parser, RBRACE_TOKEN);
            node->right = else_node;
        }
    }

    return node;
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

        /* Consume the return type after function body (e.g., } int;) */
        if (parser->token->type == ID_TOKEN)
        {
            list->datatype = type_to_type(parser->token->value);
            parser_next(parser, ID_TOKEN);
        }

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
        if (parser->token->type == RETURN_TOKEN)
        {
            list_enqueue(compound->children, parse_statement(parser));
        }
        else if (parser->token->type == IF_TOKEN)
        {
            list_enqueue(compound->children, parse_if(parser));
        }
        else 
        {
            list_enqueue(compound->children, parse_expression(parser));
        }
        if (parser->token->type == SEMI_TOKEN)
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



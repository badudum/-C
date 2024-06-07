#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "AST.h"

typedef struct PARSER_S
{
    lexer_t * lexer;
    token_t * token;
}parser_t;

parser_t * init_parser(lexer_t * lexer);

// when we call this we tell the parser that we expect a certain token, if we get an unexpected token we just DIE
token_t * parser_next(parser_t * parser, int token_type);

AST_t* parse(parser_t * parser);

AST_t * parse_factor(parser_t * parser);

AST_t * parse_term(parser_t *parser);

AST_t * parse_expression(parser_t* parser);

AST_t * parse_statement(parser_t * parser);

AST_t * parse_list(parser_t * parser);

AST_t * parse_string(parser_t * parser);

AST_t * parse_compound(parser_t * parser);
#endif
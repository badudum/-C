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
void parser_next(parser_t * parser, int token_type);

AST_t * parser_parse(parser_t* parser);

AST_t * parse_statement(parser_t* parser);

AST_t * parse_statements(parser_t* parser);

// This parse expression function will parse the basic mathematical expressions
AST_t * parse_exp(parser_t * parser);

// This function is going to parse the things that you multiply
AST_t * parse_factor(parser_t * parser);

// This fucntion is going to parse the things that you add
AST_t * parse_term(parser_t * parser);

AST_t * parse_func_call(parser_t* parser);

AST_t * parse_var(parser_t* parser);

AST_t * parse_str(parser_t* parser);

AST_t * parse_id(parser_t * parser);
#endif
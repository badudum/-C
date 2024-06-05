#ifndef LEXER_H
#define LEXER_H
#include "token.h"

/*
 * The Lexer pretty much makes sure that we have tokens that have been divided by the text from the code.
 * It will ignore whitespace, and only take the values of the needed strings.
 */


typedef struct LEXER_S
{
    char c; // current character
    unsigned int i; // index 
    char * code;
}lexer_t;

lexer_t* init_lexer(char * contents);

void lexer_getter(lexer_t* lexer);

void lexer_whitespace(lexer_t* lexer);

token_t * lexer_get_next_token(lexer_t* lexer);

token_t * lexer_advance_with_token(lexer_t* lexer, token_t * token);

token_t * lexer_collect_string(lexer_t* lexer);

token_t * lexer_collect_id(lexer_t* lexer);

char* lexer_get_current_char_as_string(lexer_t* lexer);
#endif
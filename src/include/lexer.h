#ifndef LEXER_H
#define LEXER_H
#include "token.h"
#include <stdlib.h>

/*
 * The Lexer pretty much makes sure that we have tokens that have been divided by the text from the code.
 * It will ignore whitespace, and only take the values of the needed strings.
 */


typedef struct LEXER_S
{
    char * code; //source code
    size_t code_size; // size of the source code
    char c; // current character
    unsigned int i; // index 
}lexer_t;

lexer_t* init_lexer(char * contents);

void lexer_get_next(lexer_t* lexer); //lexer_advance

char lexer_peek(lexer_t* lexer, int offset);

token_t * lexer_get_next_with_token(lexer_t* lexer, token_t * token);

token_t * lexer_get_next_with_type(lexer_t* lexer, int type);

void lexer_whitespace(lexer_t* lexer);

void lexer_comment(lexer_t* lexer);

token_t * lexer_collect_id(lexer_t* lexer);

token_t * lexer_collect_int(lexer_t* lexer);

token_t * lexer_collect_string(lexer_t* lexer);

char* lexer_get_current_char_as_string(lexer_t* lexer);

token_t * lexer_get_next_token(lexer_t* lexer);

#endif
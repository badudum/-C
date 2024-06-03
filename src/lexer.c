#include "include/lexer.h"
#include "include/token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

lexer_t *init_lexer(char *code)
{
    // We are just allocating memory and initializing the stuff
    lexer_t *lexer = calloc(1, sizeof(struct LEXER_S));
    lexer->code = code;
    lexer->i = 0;
    lexer->c = code[lexer->i];

    return lexer;
}

/*
 * this function will get the next character in the lexer
 */
void lexer_getter(lexer_t *lexer)
{
    if (lexer->c != '\0' && lexer->i < strlen(lexer->code))
    {
        lexer->i++;
        lexer->c = lexer->code[lexer->i];
    }
}

/*
 * This method makes sure that it will skip the whitespace once it encounters it
 */
void lexer_whitespace(lexer_t *lexer)
{
    // The ascii code for newline is 10
    while (lexer->c == ' ' || lexer->c == 10)
    {
        lexer_getter(lexer);
    }
}

/*
 * This function will make sure that we can distinguish between the tokens correctly
 */
token_t *lexer_get_next_token(lexer_t *lexer)
{
    while (lexer->c != '\0' && lexer->i < strlen(lexer->code))
    {
        if (lexer->c == ' ' || lexer->c == 10)
            lexer_whitespace(lexer);

        if (lexer->c == '"')
        {
            return lexer_collect_string(lexer);
        }

        if (isalnum(lexer->c))
        {
            return lexer_collect_id(lexer);
        }

        switch (lexer->c)
        {
        case '=':
            return lexer_advance_with_token(lexer, init_token(EQUALS, lexer_get_current_char_as_string(lexer)));
            break;
        case ';':
            return lexer_advance_with_token(lexer, init_token(SEMI, lexer_get_current_char_as_string(lexer)));
            break;
        case '(':
            return lexer_advance_with_token(lexer, init_token(LPAREN, lexer_get_current_char_as_string(lexer)));
            break;
        case ')':
            return lexer_advance_with_token(lexer, init_token(RPAREN, lexer_get_current_char_as_string(lexer)));
            break;
        }
    }

    return (void *)0;
}

token_t *lexer_advance_with_token(lexer_t *lexer, token_t *token)
{
    lexer_getter(lexer);
    return token;
}

token_t *lexer_collect_string(lexer_t *lexer)
{
    lexer_getter(lexer);

    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (lexer->c != '"')
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_getter(lexer);
    }

    lexer_getter(lexer);
    return init_token(STRING, value);
}

token_t *lexer_collect_id(lexer_t *lexer)
{
    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (isalnum(lexer->c)) // while the current character is alphanumeric
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_getter(lexer);
    }

    lexer_getter(lexer);
    return init_token(ID, value);
}

char *lexer_get_current_char_as_string(lexer_t *lexer)
{
    char *str = calloc(2, sizeof(char));
    str[0] = lexer->c;
    str[1] = '\0';

    return str;
}
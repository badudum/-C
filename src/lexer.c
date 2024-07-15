#include "include/lexer.h"
#include "include/token.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

lexer_t *init_lexer(char *code)
{
    // We are just allocating memory and initializing the stuff
    lexer_t *lexer = calloc(1, sizeof(struct LEXER_S));
    lexer->code = code;
    lexer->code_size = strlen(code);
    lexer->i = 0;
    lexer->c = code[lexer->i];

    return lexer;
}

/*
 * this function will get the next character in the lexer
 */
void lexer_get_next(lexer_t *lexer)
{
    if (lexer->c != '\0' && lexer->i < lexer->code_size)
    {
        lexer->i++;
        lexer->c = lexer->code[lexer->i];
    }
}

/*
 * This function will return the character at the offset and check

 */
char lexer_peek(lexer_t *lexer, int offset)
{
    if (lexer->i + offset > lexer->code_size)
        return '\0';

    return lexer->code[lexer->i + offset];
}

/*
 *  This function advaces through the lexer while keeping the current token
 */
token_t *lexer_get_next_with_token(lexer_t *lexer, token_t *token)
{
    lexer_get_next(lexer);
    return token;
}

/*
 * This function will get the next token with the type (used for short one character tokens like =, ;, (, etc)
 */
token_t *lexer_get_next_with_type(lexer_t *lexer, int type)
{
    char *value = calloc(2, sizeof(char));
    value[0] = lexer->c;
    value[1] = '\0';

    token_t *token = init_token(type, value);
    lexer_get_next(lexer);
    return token;
}

/*
 * This method makes sure that it will skip the whitespace once it encounters it
 */
void lexer_whitespace(lexer_t *lexer)
{
    // The ascii code for newline is 10 the ascii code for carriage return is 13, and the ascii code for tab is 9, but im just gonna use "\t"
    while (lexer->c == ' ' || lexer->c == 10 || lexer->c == 13 || lexer->c == '\t')
    {
        lexer_get_next(lexer);
    }
}

/*
 * This function will skip the comments (this is messed up code, ik, but its just for fun)
 */
void lexer_comment(lexer_t *lexer)
{

    // if(lexer->c == 'c')
    // {
    //     if(lexer_peek(lexer, 1) == 'o')
    //     {
    //         if(lexer_peek(lexer, 2) == 'm')
    //         {
    //             if(lexer_peek(lexer, 3) == 'm')
    //             {
    //                 if(lexer_peek(lexer, 4) == 'e')
    //                 {
    //                     if(lexer_peek(lexer, 5) == 'n')
    //                     {
    //                         if(lexer_peek(lexer, 6) == 't')
    //                         {
    //                             if(lexer_peek(lexer,7) == ':')
    //                             {
    //                                 while(1) {
    //                                     if (lexer->c == ';')
    //                                     {
    //                                         lexer_get_next(lexer);
    //                                         lexer_get_next(lexer);
    //                                         break;
    //                                     }
    //                                     lexer_get_next(lexer);
    //                                 }
    //                             }
    //                             else
    //                             {
    //                                 while(lexer->c != '\n')
    //                                 {
    //                                     lexer_get_next(lexer); // this is very bad practice, but its funny
    //                                 }
    //                             }
    //                         }
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    // better code
    int i = 0;
    char *comment = "comment";

    if (lexer->c == (char)comment[0])
    {
        i++;
        while (i <= strlen(comment))
        {
            if (lexer_peek(lexer, i) == comment[i])
            {
                i++;
                if (i == strlen(comment) - 1)
                {
                    if (lexer_peek(lexer, i + 1) == ':')
                    {
                        while (1)
                        {
                            if (lexer->c == ';')
                            {
                                lexer_get_next(lexer);
                                break;
                            }
                            lexer_get_next(lexer);
                        }
                    }
                    else
                    {
                        while (lexer->c != '\n')
                        {
                            lexer_get_next(lexer);
                        }
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    lexer_whitespace(lexer);
}

/*
 * This function creates a token with the ID value
 */

token_t *lexer_collect_id(lexer_t *lexer)
{
    int token_type = ID_TOKEN;

    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (isalnum(lexer->c)) // while the current character is alphanumeric
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_get_next(lexer);
    }

    if (strcmp(value, "return") == 0)
    {
        token_type = RETURN_TOKEN;
    }

    if(is_anagram(value, "function"))
    {
        token_type = FUNCTION_TOKEN;
    }
    return init_token(token_type, value);
}

/*
 * This function will collect the integer value
 */
token_t *lexer_collect_number(lexer_t *lexer)
{
    int token_type = INT_TOKEN;

    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (isdigit(lexer->c)) // while the current character is a number
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_get_next(lexer);
    }

    return init_token(INT_TOKEN, value);
}

/*
 * This function is here to distinguish between a string and actual code
 */
token_t *lexer_collect_string(lexer_t *lexer)
{
    lexer_get_next(lexer);

    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (lexer->c != '"')
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        lexer_get_next(lexer);
    }
    lexer_get_next(lexer);

    char* format = str_format(value);
    free(value);
    return init_token(STRING_TOKEN, format);
}

/*
 * This function will make sure that we can distinguish between the tokens correctly
 */
token_t *lexer_get_next_token(lexer_t *lexer)
{
    while (lexer->c != '\0')
    {

        lexer_whitespace(lexer);
        lexer_comment(lexer);

        if (isalpha(lexer->c))
        {
            return lexer_collect_id(lexer);
        }

        if (isdigit(lexer->c))
        {
            return lexer_collect_number(lexer);
        }

        switch (lexer->c)
        {
        case '=':
            return lexer_get_next_with_type(lexer, EQUALS_TOKEN);
        case ';':
            return lexer_get_next_with_type(lexer, SEMI_TOKEN);
        case '(':
            return lexer_get_next_with_type(lexer, LPAREN_TOKEN);
        case ')':
            return lexer_get_next_with_type(lexer, RPAREN_TOKEN);
        case '{':
            return lexer_get_next_with_type(lexer, LBRACE_TOKEN);
        case '}':
            return lexer_get_next_with_type(lexer, RBRACE_TOKEN);
        case ',':
            return lexer_get_next_with_type(lexer, COMMA_TOKEN);
        case '.':
            return lexer_get_next_with_type(lexer, DOT_TOKEN);
        case '>':
            return lexer_get_next_with_type(lexer, GT_TOKEN);
        case '<':
            return lexer_get_next_with_type(lexer, LT_TOKEN);
        case '+':
            return lexer_get_next_with_type(lexer, PLUS_TOKEN);
        case '-':
            return lexer_get_next_with_type(lexer, MINUS_TOKEN);
        case '*':
            return lexer_get_next_with_type(lexer, ASTERISK_TOKEN);
        case '/':
            return lexer_get_next_with_type(lexer, SLASH_TOKEN);
        case '[':
            return lexer_get_next_with_type(lexer, LSQUAREBRKT_TOKEN);
        case ']':
            return lexer_get_next_with_type(lexer, RSQUAREBRKT_TOKEN);
        case '"':
            return lexer_collect_string(lexer);
        case '\0':
            break;
        default:
            printf("Unexpected charactertoken: %c\n", lexer->c);
            exit(1);
            break;
        }
    }
    return init_token(EOF_TOKEN, 0);
}

/*
 * Gets the current character as a string
 */
char *lexer_get_current_char_as_string(lexer_t *lexer)
{
    char *str = calloc(2, sizeof(char));
    str[0] = lexer->c;
    str[1] = '\0';

    return str;
}
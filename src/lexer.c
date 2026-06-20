#include "include/lexer.h"
#include "include/token.h"
#include "include/errors.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static token_t *lexer_make_token(lexer_t *lexer, int type, char *value)
{
    token_t *token = init_token(type, value);
    token->line = lexer->line;
    token->column = lexer->column;
    if (lexer->filename)
        token->source_file = strdup(lexer->filename);
    return token;
}

static void lexer_advance(lexer_t *lexer)
{
    if (lexer->c != '\0' && lexer->i < lexer->code_size)
    {
        if (lexer->c == '\n') {
            lexer->line++;
            lexer->column = 1;
        } else {
            lexer->column++;
        }
        lexer->i++;
        lexer->c = lexer->code[lexer->i];
    }
}

lexer_t *init_lexer(char *code, const char *filename)
{
    lexer_t *lexer = calloc(1, sizeof(struct LEXER_S));
    lexer->code = code;
    lexer->code_size = strlen(code);
    lexer->i = 0;
    lexer->c = code[lexer->i];
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = filename ? strdup(filename) : strdup("<stdin>");

    return lexer;
}

/*
 * this function will get the next character in the lexer
 */
void lexer_get_next(lexer_t *lexer)
{
    lexer_advance(lexer);
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
    lexer_advance(lexer);
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

    token_t *token = lexer_make_token(lexer, type, value);
    lexer_advance(lexer);
    return token;
}

static int lexer_at_line_start(lexer_t *lexer)
{
    unsigned int j = lexer->i;
    while (j > 0) {
        j--;
        char c = lexer->code[j];
        if (c == '\n')
            return 1;
        if (c != ' ' && c != '\t' && c != '\r')
            return 0;
    }
    return 1;
}

static void lexer_skip_line_directive(lexer_t *lexer)
{
    /* #line <num> "file" */
    lexer_advance(lexer); /* # */
    while (lexer->c && lexer->c != '\n' && !isalpha((unsigned char)lexer->c))
        lexer_advance(lexer);

    if (strncmp(lexer->code + lexer->i, "line", 4) != 0)
        return;

    lexer->i += 4;
    lexer->c = lexer->code[lexer->i];
    while (lexer->c == ' ' || lexer->c == '\t')
        lexer_advance(lexer);

    unsigned int new_line = 0;
    while (isdigit((unsigned char)lexer->c)) {
        new_line = new_line * 10 + (unsigned int)(lexer->c - '0');
        lexer_advance(lexer);
    }
    while (lexer->c == ' ' || lexer->c == '\t')
        lexer_advance(lexer);

    if (lexer->c == '"') {
        lexer_advance(lexer);
        unsigned int start = lexer->i;
        while (lexer->c && lexer->c != '"')
            lexer_advance(lexer);
        if (lexer->c == '"') {
            size_t len = (size_t)(lexer->i - start);
            char *path = calloc(len + 1, 1);
            if (path) {
                memcpy(path, lexer->code + start, len);
                free(lexer->filename);
                lexer->filename = path;
            }
            lexer_advance(lexer);
        }
    }

    if (new_line > 0)
        lexer->line = new_line;
    lexer->column = 1;

    while (lexer->c && lexer->c != '\n')
        lexer_advance(lexer);
    if (lexer->c == '\n')
        lexer_advance(lexer);
}

/*
 * This method makes sure that it will skip the whitespace once it encounters it
 */
void lexer_whitespace(lexer_t *lexer)
{
    while (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\r')
    {
        lexer_advance(lexer);
    }
    if (lexer->c == '\n') {
        lexer_advance(lexer);
        while (lexer->c == ' ' || lexer->c == '\t' || lexer->c == '\r')
            lexer_advance(lexer);
    }
}

/*
 * This function will skip the comments (this is messed up code, ik, but its just for fun)
 */
void lexer_comment(lexer_t *lexer)
{

    int i = 0;
    char *comment = "comment";

    if (lexer->c == (char)comment[0])
    {
        i++;
        while (i <= (int)strlen(comment))
        {
            if (lexer_peek(lexer, i) == comment[i])
            {
                i++;
                if (i == (int)strlen(comment) - 1)
                {
                    if (lexer_peek(lexer, i + 1) == ':')
                    {
                        while (1)
                        {
                            if (lexer->c == ';')
                            {
                                lexer_advance(lexer);
                                break;
                            }
                            if (lexer->c == '\0')
                                break;
                            lexer_advance(lexer);
                        }
                    }
                    else
                    {
                        while (lexer->c != '\n' && lexer->c != '\0')
                        {
                            lexer_advance(lexer);
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
        free(s);
        lexer_advance(lexer);
    }

    if (strcmp(value, "return") == 0)
        token_type = RETURN_TOKEN;
    else if (is_anagram(value, "function"))
        token_type = FUNCTION_TOKEN;
    else if (strcmp(value, "if") == 0)
        token_type = IF_TOKEN;
    else if (strcmp(value, "else") == 0)
        token_type = ELSE_TOKEN;
    else if (strcmp(value, "Real") == 0)
        token_type = REAL_TOKEN;
    else if (strcmp(value, "Fake") == 0)
        token_type = FAKE_TOKEN;
    else if (strcmp(value, "and") == 0)
        token_type = AND_TOKEN;
    else if (strcmp(value, "or") == 0)
        token_type = OR_TOKEN;
    else if (strcmp(value, "not") == 0)
        token_type = NOT_TOKEN;
    else if (strcmp(value, "loop") == 0)
        token_type = LOOP_TOKEN;
    else if (strcmp(value, "until") == 0)
        token_type = UNTIL_TOKEN;
    else if (strcmp(value, "dupe") == 0)
        token_type = DUPE_TOKEN;
    else if (strcmp(value, "cust") == 0)
        token_type = CUST_TOKEN;
    else if (strcmp(value, "class") == 0)
        token_type = CLASS_TOKEN;
    else if (strcmp(value, "public") == 0)
        token_type = PUBLIC_TOKEN;
    else if (strcmp(value, "private") == 0)
        token_type = PRIVATE_TOKEN;
    else if (strcmp(value, "extends") == 0)
        token_type = EXTENDS_TOKEN;
    else if (strcmp(value, "virtual") == 0)
        token_type = VIRTUAL_TOKEN;
    else if (strcmp(value, "interface") == 0)
        token_type = INTERFACE_TOKEN;
    else if (strcmp(value, "implements") == 0)
        token_type = IMPLEMENTS_TOKEN;

    return lexer_make_token(lexer, token_type, value);
}

/*
 * This function will collect the integer value
 */
token_t *lexer_collect_number(lexer_t *lexer)
{
    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (isdigit(lexer->c)) // while the current character is a number
    {
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        free(s);
        lexer_advance(lexer);
    }

    return lexer_make_token(lexer, INT_TOKEN, value);
}

/*
 * This function is here to distinguish between a string and actual code
 */
token_t *lexer_collect_string(lexer_t *lexer)
{
    lexer_advance(lexer);

    char *value = calloc(1, sizeof(char));

    value[0] = '\0';

    while (lexer->c != '"')
    {
        if (lexer->c == '\0')
            break;
        char *s = lexer_get_current_char_as_string(lexer);
        value = realloc(value, (strlen(value) + strlen(s) + 1) * sizeof(char));
        strcat(value, s);
        free(s);
        lexer_advance(lexer);
    }
    if (lexer->c == '"')
        lexer_advance(lexer);

    char* format = str_format(value);
    free(value);
    return lexer_make_token(lexer, STRING_TOKEN, format);
}

/*
 * This function will make sure that we can distinguish between the tokens correctly
 */
token_t *lexer_get_next_token(lexer_t *lexer)
{
    while (lexer->c != '\0')
    {

        lexer_whitespace(lexer);
        if (lexer->c == '#' && lexer_at_line_start(lexer)) {
            lexer_skip_line_directive(lexer);
            continue;
        }
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
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, DEQUALS_TOKEN);
            }
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
        case ':':
            return lexer_get_next_with_type(lexer, COLON_TOKEN);
        case '!':
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, NOT_EQUALS_TOKEN);
            }
            return lexer_get_next_with_type(lexer, NOT_TOKEN);
        case '>':
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, GTE_TOKEN);
            }
            return lexer_get_next_with_type(lexer, GT_TOKEN);
        case '<':
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, LTE_TOKEN);
            }
            return lexer_get_next_with_type(lexer, LT_TOKEN);
        case '&':
            return lexer_get_next_with_type(lexer, BITAND_TOKEN);
        case '|':
            return lexer_get_next_with_type(lexer, BITOR_TOKEN);
        case '~':
            return lexer_get_next_with_type(lexer, BITNOT_TOKEN);
        case '+':
            if (lexer_peek(lexer, 1) == '+') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, PLUS_PLUS_TOKEN);
            }
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, PLUS_EQUALS_TOKEN);
            }
            return lexer_get_next_with_type(lexer, PLUS_TOKEN);
        case '-':
            if (lexer_peek(lexer, 1) == '-') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, MINUS_MINUS_TOKEN);
            }
            if (lexer_peek(lexer, 1) == '=') {
                lexer_advance(lexer);
                return lexer_get_next_with_type(lexer, MINUS_EQUALS_TOKEN);
            }
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
            compile_error_at(lexer->filename, (int)lexer->line,
                               "Unexpected character '%c'", lexer->c);
            break;
        }
    }
    return lexer_make_token(lexer, EOF_TOKEN, 0);
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

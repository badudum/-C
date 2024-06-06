#include "include/token.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

token_t * init_token(int type, char* value)
{
    token_t * token = calloc(1, sizeof(struct TOKEN_S));
    token->type = type;
    token->value = value;
    return token;
}

const char * token_type_to_string(int type)
{
    switch (type)
    {
        case ID_TOKEN : return "ID_TOKEN";
        case EQUALS_TOKEN : return "EQUALS_TOKEN";
        case SEMI_TOKEN : return "SEMI_TOKEN";
        case LPAREN_TOKEN : return "LPAREN_TOKEN"; 
        case RPAREN_TOKEN : return "RPAREN_TOKEN";
        case LBRACE_TOKEN : return "LBRACE_TOKEN";
        case RBRACE_TOKEN : return "RBRACE_TOKEN";
        case COMMA_TOKEN : return "COMMA_TOKEN";
        case DOT_TOKEN : return "DOT_TOKEN";
        case INT_TOKEN : return "INT_TOKEN";
        case GT_TOKEN : return "GT_TOKEN";
        case LT_TOKEN : return "LT_TOKEN"; 

        case EOF_TOKEN : return "EOF_TOKEN";
    }
    return "cannot become a string";
}

char * token_to_string(token_t * token)
{
    const char * type_str = token_type_to_string(token->type);
    const char * temp = "<type: %s, value: %s>";
    char * str = calloc(1, sizeof(char));
    str = realloc(str, (strlen(str) + strlen(temp) + 4) * sizeof(char));
    sprintf(str, temp, type_str, token->type, token->value);
    return str;
}

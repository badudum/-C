#ifndef TOKEN_H
#define TOKEN_H


/*
 * This is the token where it will store info about what type of token it is, as well as what the actual code segments are.
*/


typedef struct TOKEN_S
{
    enum
    {
        ID_TOKEN,
        SEMI_TOKEN,
        LPAREN_TOKEN,
        RPAREN_TOKEN,
        LBRACE_TOKEN,
        RBRACE_TOKEN,
        COMMA_TOKEN,
        DOT_TOKEN,

        IF_TOKEN,
        ELSE_TOKEN,
        RETURN_TOKEN,
        WHILE_TOKEN,
        FUNCTION_TOKEN,

        INT_TOKEN,
        STRING_TOKEN,
        BOOL_TOKEN,
        
        EQUALS_TOKEN,
        PLUS_TOKEN,
        MINUS_TOKEN,
        ASTERISK_TOKEN,
        SLASH_TOKEN,
        MODULUS_TOKEN,
        GT_TOKEN,
        LT_TOKEN,

        EOF_TOKEN
    
    }type;
    char * value;


}token_t;


token_t* init_token(int type, char* value);

const char * token_type_to_string(int type);    

char * token_to_string(token_t * token);

#endif
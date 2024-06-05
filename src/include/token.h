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
        EQUALS_TOKEN,
        STRING_TOKEN,
        SEMI_TOKEN,
        LPAREN_TOKEN,
        RPAREN_TOKEN,        
    }type;
    char * value;


}token_t;


token_t* init_token(int type, char* value);

#endif
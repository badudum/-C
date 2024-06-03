#ifndef TOKEN_H
#define TOKEN_H

typedef struct TOKEN_S
{
    enum
    {
        ID,
        EQUALS,
        STRING,
        SEMI,
        LPAREN,
        RPAREN,        
    }type;
    char * value;


}token_t;


token_t* init_token(int type, char* value);

#endif
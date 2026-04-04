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
        LSQUAREBRKT_TOKEN,
        RSQUAREBRKT_TOKEN,
        COMMA_TOKEN,
        DOT_TOKEN,
        COLON_TOKEN,

        IF_TOKEN,
        ELSE_TOKEN,
        RETURN_TOKEN,
        WHILE_TOKEN,
        LOOP_TOKEN,
        UNTIL_TOKEN,
        FUNCTION_TOKEN,

        INT_TOKEN,
        STRING_TOKEN,
        BOOL_TOKEN,
        REAL_TOKEN,
        FAKE_TOKEN,
        
        EQUALS_TOKEN,
        PLUS_EQUALS_TOKEN,
        MINUS_EQUALS_TOKEN,
        DEQUALS_TOKEN,
        NOT_EQUALS_TOKEN,
        PLUS_TOKEN,
        MINUS_TOKEN,
        PLUS_PLUS_TOKEN,
        MINUS_MINUS_TOKEN,
        ASTERISK_TOKEN,
        SLASH_TOKEN,
        MODULUS_TOKEN,
        GT_TOKEN,
        LT_TOKEN,
        GTE_TOKEN,
        LTE_TOKEN,

        AND_TOKEN,
        OR_TOKEN,
        NOT_TOKEN,
        BITAND_TOKEN,
        BITOR_TOKEN,
        BITNOT_TOKEN,

        EOF_TOKEN
    
    }type;
    char * value;


}token_t;


token_t* init_token(int type, char* value);

const char * token_type_to_string(int type);    

char * token_to_string(token_t * token);

#endif
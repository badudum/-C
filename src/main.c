#include <stdio.h>
#include "include/lexer.h"

int main(int argc, char* argv[])
{
    lexer_t * lexer= init_lexer(
        "var name = \"john do\";\n"
        "print(name);\n"
    );
    token_t * token = (void*)0;
    while ((token = lexer_get_next_token(lexer)) != (void*)0)
    {
        printf("TOKEN(%d, %s)\n", token->type, token->value);
    }
    return 0;
}
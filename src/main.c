#include <stdio.h>
#include "include/lexer.h"
#include "include/minusC.h"

// int main(int argc, char* argv[])
// {
//     lexer_t * lexer= init_lexer(

//         "main() {"
//             "{name} str = \"john do\";" 
//             "HelloWorld(name);"
//         "} int;"
//     );
//     token_t * token = (void*)0;
//     while ((token = lexer_get_next_token(lexer)) != (void*)0)
//     {
//         printf("TOKEN(%d, %s)\n", token->type, token->value);
//     }
//     return 0;
// }


int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Please specify an input file\n");
        return 1;
    }

    minusCompile_file(argv[1]);
}

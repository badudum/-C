#include "../include/errors.h"
#include <stdio.h>

void check_arguments(AST_t* caller, AST_t* func)
{
    int expected_args_num = func->children->size;
    int actual_args_num = caller->children->size;

    if (expected_args_num != actual_args_num)
    {
        fprintf(stderr, "Error: Expected %d arguments, but got %d\n", expected_args_num, actual_args_num);
        exit(1);
    }
}
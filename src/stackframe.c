#include "include/stackframe.h"

stackframe_t * init_stackframe()
{
    stackframe_t * stackframe = calloc(1, sizeof(struct STACKFRAME_S));
    stackframe->stack = init_list(sizeof(char*));
    return stackframe;
}
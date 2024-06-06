#ifndef STACKFRAME_H
#define STACKFRAME_H
#include "list.h"
typedef struct STACKFRAME_S
{
    dynamic_list_t * stack;
}stackframe_t;

stackframe_t * init_stackframe();
#endif
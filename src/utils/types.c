#include "../include/types.h"
#include <string.h>

int type_to_type(const char* name)
{
    if (strcmp(name, "int") == 0)
    {
        return TYPE_INT;
    }
    else if (strcmp(name, "bool") == 0)
    {
        return TYPE_BOOL;
    }
    return TYPE_UNKNOWN;
}

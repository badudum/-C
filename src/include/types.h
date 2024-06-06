#ifndef TYPES_H
#define TYPES_H


typedef enum
{
    TYPE_INT = 3,
    TYPE_BOOL = 4,
    TYPE_UNKNOWN
}datatype;

int type_to_type(const char* name);

#endif

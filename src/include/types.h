#ifndef TYPES_H
#define TYPES_H


typedef enum
{
    TYPE_INT = 3,
    TYPE_BOOL = 4,
    TYPE_STR = 5,
    TYPE_UNKNOWN = 6,
    TYPE_ARRAY = 10,
}datatype;

#define IS_ARRAY_TYPE(dt) ((dt) >= TYPE_ARRAY)
#define ARRAY_ELEM_TYPE(dt) ((dt) - TYPE_ARRAY)

int type_to_type(const char* name);

#endif

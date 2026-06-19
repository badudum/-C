#ifndef TYPES_H
#define TYPES_H


typedef enum
{
    TYPE_INT = 3,
    TYPE_BOOL = 4,
    TYPE_STR = 5,
    TYPE_UNKNOWN = 6,
    TYPE_ARRAY = 10,
    TYPE_ADR = 20,
    TYPE_CUST = 100,
}datatype;

#define IS_ARRAY_TYPE(dt) ((dt) >= TYPE_ARRAY && (dt) < TYPE_ADR)
#define ARRAY_ELEM_TYPE(dt) ((dt) - TYPE_ARRAY)
#define IS_CUST_TYPE(dt) ((dt) >= TYPE_CUST)
#define CUST_TYPE_ID(dt) ((dt) - TYPE_CUST)
#define MAKE_CUST_TYPE(id) (TYPE_CUST + (id))

int type_to_type(const char* name);
int resolve_type_name(const char *name);
int datatype_heap_size(int dt);

#endif

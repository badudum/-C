#ifndef TYPES_H
#define TYPES_H


typedef enum
{
    TYPE_INT = 3,
    TYPE_BOOL = 4,
    TYPE_STR = 5,
    TYPE_UNKNOWN = 6,
    TYPE_ARRAY = 10,
    TYPE_TYPE_PARAM = 23,
    TYPE_INTERFACE = 30,
    TYPE_ADR = 20,
    TYPE_ADR_SHREF = 21,
    TYPE_ADR_MUTREF = 22,
    TYPE_CUST = 100,
}datatype;

#define IS_TYPE_PARAM(dt) ((dt) >= TYPE_TYPE_PARAM && (dt) < TYPE_INTERFACE)
#define TYPE_PARAM_INDEX(dt) ((int)((dt) - TYPE_TYPE_PARAM))
#define MAKE_TYPE_PARAM(i) (TYPE_TYPE_PARAM + (i))
#define TYPE_PARAM_MAX 7

#define IS_INTERFACE_TYPE(dt) ((dt) >= TYPE_INTERFACE && (dt) < TYPE_CUST)
#define INTERFACE_TYPE_ID(dt) ((int)((dt) - TYPE_INTERFACE))
#define MAKE_INTERFACE_TYPE(id) (TYPE_INTERFACE + (id))

#define IS_ARRAY_TYPE(dt) ((dt) >= TYPE_ARRAY && (dt) < TYPE_ADR)
#define ARRAY_ELEM_TYPE(dt) ((dt) - TYPE_ARRAY)
#define IS_CUST_TYPE(dt) ((dt) >= TYPE_CUST)
#define CUST_TYPE_ID(dt) ((dt) - TYPE_CUST)
#define MAKE_CUST_TYPE(id) (TYPE_CUST + (id))
#define IS_ADR_REF(dt) ((dt) == TYPE_ADR_SHREF || (dt) == TYPE_ADR_MUTREF)
#define IS_ADR_OWNER(dt) ((dt) == TYPE_ADR)
#define IS_ADR_LIKE(dt) (IS_ADR_OWNER(dt) || IS_ADR_REF(dt))
#define HEAP_CUST_TAG_NONE (-1)
#define IS_HEAP_CUST_TAG(tag) ((tag) >= 0)
#define IS_HEAP_CUST_VAR(dt, tag) (IS_ADR_OWNER(dt) && IS_HEAP_CUST_TAG(tag))

int type_to_type(const char* name);
int resolve_type_name(const char *name);
int resolve_type_name_with_interface(const char *name);
int datatype_heap_size(int dt);
const char *datatype_mangle_suffix(int dt);

#endif

#include "../include/types.h"
#include "../include/cust.h"
#include "../include/interface.h"
#include "../include/generic.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
    else if (strcmp(name, "str") == 0)
    {
        return TYPE_STR;
    }
    else if (strcmp(name, "adr") == 0)
    {
        return TYPE_ADR;
    }
    else if (strcmp(name, "Array") == 0)
    {
        return TYPE_ARRAY;
    }
    return TYPE_UNKNOWN;
}

int resolve_type_name_with_interface(const char *name)
{
    int dt = type_to_type(name);
    if (dt != TYPE_UNKNOWN)
        return dt;
    int iid = interface_lookup_by_name(name);
    if (iid >= 0)
        return MAKE_INTERFACE_TYPE(iid);
    int cid = cust_lookup_by_name(name);
    if (cid >= 0)
        return MAKE_CUST_TYPE(cid);
    return TYPE_UNKNOWN;
}

int resolve_type_name(const char *name)
{
    return resolve_type_name_with_interface(name);
}

const char *datatype_mangle_suffix(int dt)
{
    if (dt == TYPE_INT)
        return "int";
    if (dt == TYPE_BOOL)
        return "bool";
    if (dt == TYPE_STR)
        return "str";
    if (dt == TYPE_ADR)
        return "adr";
    if (IS_ARRAY_TYPE(dt))
        return "Array";
    if (IS_CUST_TYPE(dt)) {
        cust_type_t *t = cust_get(CUST_TYPE_ID(dt));
        static char buf[128];
        if (t && t->name) {
            snprintf(buf, sizeof(buf), "%s", t->name);
            return buf;
        }
        return "cust";
    }
    return "unknown";
}

int datatype_heap_size(int dt)
{
    if (dt == TYPE_INT)
        return 4;
    if (dt == TYPE_BOOL)
        return 1;
    if (dt == TYPE_STR)
        return 1;
    if (dt == TYPE_ADR)
        return 8;
    if (IS_ARRAY_TYPE(dt))
        return datatype_heap_size(ARRAY_ELEM_TYPE(dt));
    if (IS_CUST_TYPE(dt))
        return cust_type_size(CUST_TYPE_ID(dt));
    return 0;
}

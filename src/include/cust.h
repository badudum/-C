#ifndef CUST_H
#define CUST_H

#include "types.h"
#include "list.h"

typedef struct
{
    char *name;
    int datatype;
    int offset;
} cust_field_t;

typedef struct
{
    char *name;
    int id;
    int size;
    dynamic_list_t *fields; /* cust_field_t* */
} cust_type_t;

void cust_registry_reset(void);
int cust_type_count(void);
int cust_lookup_by_name(const char *name);
cust_type_t *cust_get(int id);
int cust_field_index(int type_id, const char *field_name);
cust_field_t *cust_field_by_name(int type_id, const char *field_name);
int cust_type_size(int type_id);
int cust_type_slots(int type_id);
int cust_register_from_ast(const char *name, dynamic_list_t *field_nodes);

#endif

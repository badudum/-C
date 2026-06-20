#ifndef CUST_H
#define CUST_H

#include "types.h"
#include "list.h"
#include "AST.h"

#define CUST_VIS_PUBLIC 0
#define CUST_VIS_PRIVATE 1
#define CUST_ACCESS_STACK 0
#define CUST_ACCESS_HEAP 1
#define CUST_CALL_VIRTUAL 2
#define CUST_CALL_SUPER 3

typedef struct
{
    char *name;
    int datatype;
    int offset;
    int visibility;
    int declaring_type_id;
} cust_field_t;

typedef struct
{
    char *name;
    char *mangled_name;
    int return_type;
    int visibility;
    int is_instance;
    int is_virtual;
    int is_override;
    int vtable_slot;
} cust_method_t;

typedef struct
{
    char *name;
    int id;
    int size;
    int base_type_id;
    int has_vtable;
    int vtable_slots;
    dynamic_list_t *fields; /* cust_field_t* */
    dynamic_list_t *methods; /* cust_method_t* */
} cust_type_t;

void cust_registry_reset(void);
int cust_type_count(void);
int cust_lookup_by_name(const char *name);
cust_type_t *cust_get(int id);
int cust_base_type_id(int type_id);
int cust_is_subtype(int type_id, int base_type_id);
int cust_field_index(int type_id, const char *field_name);
cust_field_t *cust_field_by_name(int type_id, const char *field_name);
cust_field_t *cust_field_lookup(int type_id, const char *field_name, int *declaring_type_id);
cust_method_t *cust_method_by_name(int type_id, const char *method_name);
cust_method_t *cust_method_lookup(int type_id, const char *method_name);
char *cust_mangle_method(const char *type_name, const char *method_name);
int cust_type_size(int type_id);
int cust_heap_header_size(int type_id);
int cust_heap_object_size(int type_id);
int cust_type_slots(int type_id);
int cust_register_from_ast(const AST_t *def, int base_type_id, dynamic_list_t *implements_ifaces);
char *cust_emit_vtables(void);

#endif

#ifndef INTERFACE_H
#define INTERFACE_H

#include "types.h"
#include "list.h"
#include "AST.h"

typedef struct
{
    char *name;
    int return_type;
    int vtable_slot;
    /* Unified cust vtable slot for dispatch; set when first type implements. */
    int dispatch_slot;
} interface_method_t;

typedef struct
{
    char *name;
    int id;
    dynamic_list_t *methods; /* interface_method_t* */
} interface_type_t;

void interface_registry_reset(void);
int interface_lookup_by_name(const char *name);
interface_type_t *interface_get(int id);
interface_method_t *interface_method_by_name(int interface_id, const char *method_name);

void interface_register_from_ast(const char *name, dynamic_list_t *member_nodes,
                                 const AST_t *loc);

/* Verify cust type satisfies interface; exits on failure. */
void interface_check_implements(int cust_id, int interface_id, const AST_t *loc);

int interface_cust_implements(int cust_id, int interface_id);

void interface_cust_add(int cust_id, int interface_id);

#endif

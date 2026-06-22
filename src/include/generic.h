#ifndef GENERIC_H
#define GENERIC_H

#include "types.h"
#include "list.h"
#include "AST.h"

void generic_registry_reset(void);
int generic_template_count(void);
int generic_lookup_template(const char *name);

/* Register Box<T> = cust { ... }; def_ast is the cust body (CUST_DEF_AST children). */
void generic_register_template(const char *name, dynamic_list_t *param_names,
                               dynamic_list_t *member_nodes,
                               dynamic_list_t *iface_ids, int base_type_id,
                               const AST_t *loc);

int generic_instantiate(const char *name, dynamic_list_t *arg_dts, const AST_t *loc);

/* Append monomorphized cust defs (with methods) to the parse root for codegen. */
void generic_append_instantiated_to_root(AST_t *root);

#endif

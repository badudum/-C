#ifndef MODULE_H
#define MODULE_H

#include "list.h"

void module_registry_reset(void);

/* Register a file included via `reference qualified <path>`. */
void module_register_qualified(const char *full_path, const char *module_name);

/* Return module name when source_file belongs to a qualified module, else NULL. */
const char *module_name_for_source(const char *source_file);

int module_is_name(const char *name);

unsigned int module_registered_count(void);
const char *module_registered_name(unsigned int index);

/* module__symbol (caller frees). */
char *module_mangle_symbol(const char *module_name, const char *symbol);

/* Basename of ref path without .minusc (caller frees). */
char *module_name_from_ref_path(const char *ref_path);

#endif

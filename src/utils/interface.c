#include "../include/interface.h"
#include "../include/generic.h"
#include "../include/cust.h"
#include "../include/errors.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dynamic_list_t *interface_registry;
static dynamic_list_t *cust_interfaces; /* list of dynamic_list_t* per cust id */

void interface_registry_reset(void)
{
    if (interface_registry) {
        for (unsigned int i = 0; i < interface_registry->size; i++) {
            interface_type_t *it = (interface_type_t *)interface_registry->items[i];
            if (!it)
                continue;
            free(it->name);
            if (it->methods) {
                for (unsigned int j = 0; j < it->methods->size; j++) {
                    interface_method_t *m = (interface_method_t *)it->methods->items[j];
                    free(m->name);
                    free(m);
                }
                list_free(it->methods);
            }
            free(it);
        }
        list_free(interface_registry);
    }
    interface_registry = 0;
    if (cust_interfaces) {
        for (unsigned int i = 0; i < cust_interfaces->size; i++) {
            dynamic_list_t *lst = (dynamic_list_t *)cust_interfaces->items[i];
            if (lst)
                list_free(lst);
        }
        list_free(cust_interfaces);
    }
    cust_interfaces = 0;
}

int interface_lookup_by_name(const char *name)
{
    if (!name || !interface_registry)
        return -1;
    for (unsigned int i = 0; i < interface_registry->size; i++) {
        interface_type_t *it = (interface_type_t *)interface_registry->items[i];
        if (it && it->name && strcmp(it->name, name) == 0)
            return (int)i;
    }
    return -1;
}

interface_type_t *interface_get(int id)
{
    if (!interface_registry || id < 0 || (unsigned int)id >= interface_registry->size)
        return 0;
    return (interface_type_t *)interface_registry->items[id];
}

interface_method_t *interface_method_by_name(int interface_id, const char *method_name)
{
    interface_type_t *it = interface_get(interface_id);
    if (!it || !method_name || !it->methods)
        return 0;
    for (unsigned int i = 0; i < it->methods->size; i++) {
        interface_method_t *im = (interface_method_t *)it->methods->items[i];
        if (im && im->name && strcmp(im->name, method_name) == 0)
            return im;
    }
    return 0;
}

static dynamic_list_t *cust_iface_list(int cust_id)
{
    if (cust_id < 0)
        return 0;
    if (!cust_interfaces)
        cust_interfaces = init_list(sizeof(dynamic_list_t *));
    while ((int)cust_interfaces->size <= cust_id)
        list_enqueue(cust_interfaces, 0);
    dynamic_list_t *lst = (dynamic_list_t *)cust_interfaces->items[cust_id];
    if (!lst) {
        lst = init_list(sizeof(void *));
        cust_interfaces->items[cust_id] = lst;
    }
    return lst;
}

void interface_cust_add(int cust_id, int interface_id)
{
    dynamic_list_t *lst = cust_iface_list(cust_id);
    if (!lst)
        return;
    for (unsigned int i = 0; i < lst->size; i++) {
        if ((int)(intptr_t)lst->items[i] == interface_id)
            return;
    }
    list_enqueue(lst, (void *)(intptr_t)interface_id);
}

int interface_cust_implements(int cust_id, int interface_id)
{
    if (cust_id < 0 || interface_id < 0 || !cust_interfaces)
        return 0;
    if ((unsigned int)cust_id >= cust_interfaces->size)
        return 0;
    dynamic_list_t *lst = (dynamic_list_t *)cust_interfaces->items[cust_id];
    if (!lst)
        return 0;
    for (unsigned int i = 0; i < lst->size; i++) {
        if ((int)(intptr_t)lst->items[i] == interface_id)
            return 1;
    }
    return 0;
}

void interface_register_from_ast(const char *name, dynamic_list_t *member_nodes,
                                 const AST_t *loc)
{
    if (!name || !member_nodes) {
        compile_error_ast(loc, "invalid interface definition");
    }
    if (interface_lookup_by_name(name) >= 0) {
        compile_error_ast(loc, "interface '%s' already defined", name);
    }
    if (cust_lookup_by_name(name) >= 0 || generic_lookup_template(name) >= 0) {
        compile_error_ast(loc, "interface name '%s' conflicts with existing type", name);
    }
    if (!interface_registry)
        interface_registry = init_list(sizeof(interface_type_t *));

    interface_type_t *it = calloc(1, sizeof(interface_type_t));
    it->name = strdup(name);
    it->id = (int)interface_registry->size;
    it->methods = init_list(sizeof(interface_method_t *));

    int slot = 0;
    for (unsigned int i = 0; i < member_nodes->size; i++) {
        AST_t *node = (AST_t *)member_nodes->items[i];
        if (!node || node->type != FUNC_AST || !node->name) {
            compile_error_ast(loc, "interface '%s' requires method signatures", name);
        }
        char *short_name = strrchr(node->name, '_');
        char *method_name = short_name && short_name[1]
            ? strdup(short_name + 1)
            : strdup(node->name);
        interface_method_t *im = calloc(1, sizeof(interface_method_t));
        im->name = method_name;
        im->return_type = node->datatype;
        im->vtable_slot = slot++;
        im->dispatch_slot = -1;
        list_enqueue(it->methods, im);
    }
    list_enqueue(interface_registry, it);
}

void interface_check_implements(int cust_id, int interface_id, const AST_t *loc)
{
    interface_type_t *iface = interface_get(interface_id);
    cust_type_t *cust = cust_get(cust_id);
    if (!iface || !cust) {
        compile_error_ast(loc, "internal error checking interface implementation");
    }

    for (unsigned int i = 0; i < iface->methods->size; i++) {
        interface_method_t *im = (interface_method_t *)iface->methods->items[i];
        if (!im)
            continue;
        cust_method_t *cm = cust_method_lookup(cust_id, im->name);
        if (!cm) {
            compile_error_ast(loc, "'%s' does not implement interface method '%s'",
                              cust->name, im->name);
        }
        if (!cm->is_virtual) {
            compile_error_ast(loc,
                              "interface method '%s' on '%s' must be declared 'virtual'",
                              im->name, cust->name);
        }
        if (cm->return_type != im->return_type) {
            compile_error_ast(loc,
                              "interface method '%s' on '%s' has mismatched return type",
                              im->name, cust->name);
        }
        if (!cm->is_virtual || cm->vtable_slot < 0) {
            compile_error_ast(loc,
                              "interface method '%s' on '%s' must be declared 'virtual'",
                              im->name, cust->name);
        }
        if (im->dispatch_slot < 0)
            im->dispatch_slot = cm->vtable_slot;
        else if (im->dispatch_slot != cm->vtable_slot) {
            compile_error_ast(loc,
                              "'%s' implements '%s.%s' at vtable slot %d but requires slot %d",
                              cust->name, iface->name, im->name,
                              cm->vtable_slot, im->dispatch_slot);
        }
    }
    interface_cust_add(cust_id, interface_id);
}

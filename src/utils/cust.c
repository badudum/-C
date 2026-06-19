#include "../include/cust.h"
#include "../include/AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static dynamic_list_t *cust_registry;

static int align_up(int offset, int align)
{
    if (align <= 1)
        return offset;
    int rem = offset % align;
    return rem ? offset + (align - rem) : offset;
}

static int field_align(int dt)
{
    if (IS_CUST_TYPE(dt))
        return 8;
    if (dt == TYPE_ADR || dt == TYPE_STR)
        return 8;
    if (IS_ARRAY_TYPE(dt))
        return 4;
    if (dt == TYPE_INT || dt == TYPE_BOOL)
        return 4;
    return 4;
}

static int field_size_for_datatype(int dt)
{
    if (IS_CUST_TYPE(dt))
        return cust_type_size(CUST_TYPE_ID(dt));
    if (dt == TYPE_STR || dt == TYPE_ADR)
        return 8;
    if (IS_ARRAY_TYPE(dt))
        return 4;
    return datatype_heap_size(dt);
}

void cust_registry_reset(void)
{
    if (!cust_registry) {
        cust_registry = init_list(sizeof(cust_type_t *));
        return;
    }
    for (unsigned int i = 0; i < cust_registry->size; i++) {
        cust_type_t *t = (cust_type_t *)cust_registry->items[i];
        if (!t)
            continue;
        for (unsigned int j = 0; j < t->fields->size; j++) {
            cust_field_t *f = (cust_field_t *)t->fields->items[j];
            if (f) {
                free(f->name);
                free(f);
            }
        }
        free(t->fields->items);
        free(t->fields);
        free(t->name);
        free(t);
    }
    cust_registry->size = 0;
}

int cust_type_count(void)
{
    return cust_registry ? (int)cust_registry->size : 0;
}

int cust_lookup_by_name(const char *name)
{
    if (!name || !cust_registry)
        return -1;
    for (unsigned int i = 0; i < cust_registry->size; i++) {
        cust_type_t *t = (cust_type_t *)cust_registry->items[i];
        if (t && t->name && strcmp(t->name, name) == 0)
            return (int)i;
    }
    return -1;
}

cust_type_t *cust_get(int id)
{
    if (!cust_registry || id < 0 || (unsigned int)id >= cust_registry->size)
        return 0;
    return (cust_type_t *)cust_registry->items[id];
}

int cust_field_index(int type_id, const char *field_name)
{
    cust_field_t *f = cust_field_by_name(type_id, field_name);
    if (!f)
        return -1;
    return f->offset;
}

cust_field_t *cust_field_by_name(int type_id, const char *field_name)
{
    cust_type_t *t = cust_get(type_id);
    if (!t || !field_name)
        return 0;
    for (unsigned int i = 0; i < t->fields->size; i++) {
        cust_field_t *f = (cust_field_t *)t->fields->items[i];
        if (f && f->name && strcmp(f->name, field_name) == 0)
            return f;
    }
    return 0;
}

int cust_type_size(int type_id)
{
    cust_type_t *t = cust_get(type_id);
    return t ? t->size : 0;
}

int cust_type_slots(int type_id)
{
    int sz = cust_type_size(type_id);
    if (sz <= 0)
        return 1;
    return (sz + 15) / 16;
}

int cust_register_from_ast(const char *name, dynamic_list_t *field_nodes)
{
    if (!name || !field_nodes) {
        fprintf(stderr, "Error: invalid cust definition\n");
        exit(1);
    }
    if (cust_lookup_by_name(name) >= 0) {
        fprintf(stderr, "Error: cust type '%s' already defined\n", name);
        exit(1);
    }
    if (!cust_registry)
        cust_registry = init_list(sizeof(cust_type_t *));

    cust_type_t *type = calloc(1, sizeof(cust_type_t));
    type->name = strdup(name);
    type->id = (int)cust_registry->size;
    type->fields = init_list(sizeof(cust_field_t *));
    int offset = 0;

    for (unsigned int i = 0; i < field_nodes->size; i++) {
        AST_t *node = (AST_t *)field_nodes->items[i];
        if (!node || node->type != VAR_AST || !node->name) {
            fprintf(stderr, "Error: invalid field in cust '%s'\n", name);
            exit(1);
        }
        if (!node->datatype || node->datatype == TYPE_UNKNOWN) {
            fprintf(stderr, "Error: unknown field type for '%s' in cust '%s'\n",
                    node->name, name);
            exit(1);
        }
        if (IS_ARRAY_TYPE(node->datatype) && node->datatype == TYPE_ARRAY) {
            fprintf(stderr, "Error: Array fields need element type in cust '%s'\n", name);
            exit(1);
        }

        int align = field_align(node->datatype);
        offset = align_up(offset, align);
        cust_field_t *field = calloc(1, sizeof(cust_field_t));
        field->name = strdup(node->name);
        field->datatype = node->datatype;
        field->offset = offset;
        offset += field_size_for_datatype(node->datatype);
        list_enqueue(type->fields, field);
    }

    offset = align_up(offset, 8);
    type->size = offset;
    list_enqueue(cust_registry, type);
    return type->id;
}

#include "../include/cust.h"
#include "../include/AST.h"
#include "../include/errors.h"
#include "../include/interface.h"
#include "../include/numeric.h"
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
    if (IS_NUMERIC_TYPE(dt))
        return numeric_bit_width(dt) >= 64 ? 8 : 4;
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

static cust_method_t *method_in_type_list(cust_type_t *t, const char *method_name)
{
    if (!t || !method_name || !t->methods)
        return 0;
    for (unsigned int i = 0; i < t->methods->size; i++) {
        cust_method_t *m = (cust_method_t *)t->methods->items[i];
        if (m && m->name && strcmp(m->name, method_name) == 0)
            return m;
    }
    return 0;
}

static cust_method_t *method_in_type(int type_id, const char *method_name)
{
    return method_in_type_list(cust_get(type_id), method_name);
}

static cust_method_t *method_in_base_chain(int type_id, const char *method_name)
{
    while (type_id >= 0) {
        cust_method_t *m = method_in_type(type_id, method_name);
        if (m)
            return m;
        type_id = cust_base_type_id(type_id);
    }
    return 0;
}

static void cust_build_vtable(cust_type_t *type)
{
    if (!type)
        return;
    int base_id = type->base_type_id;
    cust_type_t *base = base_id >= 0 ? cust_get(base_id) : 0;
    if (base && base->has_vtable) {
        type->has_vtable = 1;
        type->vtable_slots = base->vtable_slots;
    }

    for (unsigned int i = 0; i < type->methods->size; i++) {
        cust_method_t *m = (cust_method_t *)type->methods->items[i];
        if (!m || !m->is_virtual)
            continue;
        cust_method_t *base_m = 0;
        if (base_id >= 0)
            base_m = method_in_base_chain(base_id, m->name);
        if (base_m && base_m->is_virtual) {
            if (base_m->return_type != m->return_type) {
                compile_error_at(NULL, 0,
                    "override of '%s' in '%s' has mismatched return type",
                    m->name, type->name);
            }
            m->is_override = 1;
            m->vtable_slot = base_m->vtable_slot;
        } else {
            m->vtable_slot = type->vtable_slots++;
            type->has_vtable = 1;
        }
    }
}

char *cust_mangle_method(const char *type_name, const char *method_name)
{
    if (!type_name || !method_name)
        return 0;
    size_t len = strlen(type_name) + strlen(method_name) + 2;
    char *buf = calloc(len, sizeof(char));
    snprintf(buf, len, "%s_%s", type_name, method_name);
    return buf;
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
        if (t->methods) {
            for (unsigned int j = 0; j < t->methods->size; j++) {
                cust_method_t *m = (cust_method_t *)t->methods->items[j];
                if (m) {
                    free(m->name);
                    free(m->mangled_name);
                    free(m);
                }
            }
            free(t->methods->items);
            free(t->methods);
        }
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

int cust_base_type_id(int type_id)
{
    cust_type_t *t = cust_get(type_id);
    return t ? t->base_type_id : -1;
}

int cust_is_subtype(int type_id, int base_type_id)
{
    while (type_id >= 0) {
        if (type_id == base_type_id)
            return 1;
        type_id = cust_base_type_id(type_id);
    }
    return 0;
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
    int decl = -1;
    return cust_field_lookup(type_id, field_name, &decl);
}

cust_field_t *cust_field_lookup(int type_id, const char *field_name, int *declaring_type_id)
{
    if (declaring_type_id)
        *declaring_type_id = -1;
    cust_type_t *t = cust_get(type_id);
    if (!t || !field_name)
        return 0;
    for (unsigned int i = 0; i < t->fields->size; i++) {
        cust_field_t *f = (cust_field_t *)t->fields->items[i];
        if (f && f->name && strcmp(f->name, field_name) == 0) {
            if (declaring_type_id)
                *declaring_type_id = f->declaring_type_id;
            return f;
        }
    }
    return 0;
}

cust_method_t *cust_method_by_name(int type_id, const char *method_name)
{
    return method_in_type(type_id, method_name);
}

cust_method_t *cust_method_lookup(int type_id, const char *method_name)
{
    return method_in_base_chain(type_id, method_name);
}

int cust_type_size(int type_id)
{
    cust_type_t *t = cust_get(type_id);
    return t ? t->size : 0;
}

int cust_heap_header_size(int type_id)
{
    cust_type_t *t = cust_get(type_id);
    return (t && t->has_vtable) ? 8 : 0;
}

int cust_heap_object_size(int type_id)
{
    return cust_type_size(type_id) + cust_heap_header_size(type_id);
}

int cust_type_slots(int type_id)
{
    int sz = cust_type_size(type_id);
    if (sz <= 0)
        return 1;
    return (sz + 15) / 16;
}

static void copy_base_fields(cust_type_t *type, cust_type_t *base)
{
    for (unsigned int i = 0; i < base->fields->size; i++) {
        cust_field_t *bf = (cust_field_t *)base->fields->items[i];
        if (!bf)
            continue;
        cust_field_t *field = calloc(1, sizeof(cust_field_t));
        field->name = strdup(bf->name);
        field->datatype = bf->datatype;
        field->offset = bf->offset;
        field->visibility = bf->visibility;
        field->declaring_type_id = bf->declaring_type_id;
        list_enqueue(type->fields, field);
    }
}

static void copy_base_methods(cust_type_t *type, cust_type_t *base)
{
    for (unsigned int i = 0; i < base->methods->size; i++) {
        cust_method_t *bm = (cust_method_t *)base->methods->items[i];
        if (!bm)
            continue;
        cust_method_t *method = calloc(1, sizeof(cust_method_t));
        method->name = strdup(bm->name);
        method->mangled_name = strdup(bm->mangled_name);
        method->return_type = bm->return_type;
        method->visibility = bm->visibility;
        method->is_instance = bm->is_instance;
        method->is_virtual = bm->is_virtual;
        method->is_override = 0;
        method->vtable_slot = bm->vtable_slot;
        list_enqueue(type->methods, method);
    }
}

int cust_register_forward(const char *name, const AST_t *loc)
{
    if (!name) {
        compile_error_ast(loc, "invalid forward cust name");
    }
    if (cust_lookup_by_name(name) >= 0) {
        return cust_lookup_by_name(name);
    }
    if (!cust_registry)
        cust_registry = init_list(sizeof(cust_type_t *));
    cust_type_t *type = calloc(1, sizeof(cust_type_t));
    type->name = strdup(name);
    type->id = (int)cust_registry->size;
    type->base_type_id = -1;
    type->fields = init_list(sizeof(cust_field_t *));
    type->methods = init_list(sizeof(cust_method_t *));
    type->is_forward = 1;
    type->size = 0;
    list_enqueue(cust_registry, type);
    return type->id;
}

int cust_register_from_ast(const AST_t *def, int base_type_id, dynamic_list_t *implements_ifaces)
{
    if (!def || !def->name || !def->children) {
        compile_error_ast(def, "invalid cust definition");
    }
    int existing_id = cust_lookup_by_name(def->name);
    cust_type_t *type = 0;
    int completing_forward = 0;
    if (existing_id >= 0) {
        type = cust_get(existing_id);
        if (!type || !type->is_forward) {
            compile_error_ast(def, "cust type '%s' already defined", def->name);
        }
        completing_forward = 1;
        type->is_forward = 0;
        type->base_type_id = base_type_id;
        list_free_shallow(type->fields);
        list_free_shallow(type->methods);
        type->fields = init_list(sizeof(cust_field_t *));
        type->methods = init_list(sizeof(cust_method_t *));
    } else {
        if (!cust_registry)
            cust_registry = init_list(sizeof(cust_type_t *));
    }

    cust_type_t *base = 0;
    if (base_type_id >= 0) {
        base = cust_get(base_type_id);
        if (!base) {
            compile_error_ast(def, "unknown base type for '%s'", def->name);
        }
    }

    if (!type) {
        type = calloc(1, sizeof(cust_type_t));
        type->name = strdup(def->name);
        type->id = (int)cust_registry->size;
        type->base_type_id = base_type_id;
        type->fields = init_list(sizeof(cust_field_t *));
        type->methods = init_list(sizeof(cust_method_t *));
    }
    int offset = 0;

    if (base) {
        copy_base_fields(type, base);
        copy_base_methods(type, base);
        offset = base->size;
    }

    for (unsigned int i = 0; i < def->children->size; i++) {
        AST_t *node = (AST_t *)def->children->items[i];
        if (!node)
            continue;

        if (node->type == FUNC_AST) {
            if (!node->name) {
                compile_error_ast(def, "method missing name in cust '%s'", def->name);
            }
            char *short_name = NULL;
            if (strstr(node->name, "_operator_")) {
                char *p = strstr(node->name, "_operator_");
                short_name = strdup(p + 1);
            } else {
                char *underscore = strrchr(node->name, '_');
                short_name = underscore && underscore[1]
                    ? strdup(underscore + 1)
                    : strdup(node->name);
            }
            char *method_name = short_name;
            cust_method_t *existing = method_in_type_list(type, method_name);
            cust_method_t *base_m = base ? method_in_base_chain(base_type_id, method_name) : 0;
            if (existing) {
                int can_override = 0;
                if (node->stack_index && base_m && base_m->is_virtual)
                    can_override = 1;
                else if (!node->stack_index && base_m && !base_m->is_virtual)
                    can_override = 1;
                if (can_override) {
                    if (existing->return_type != node->datatype) {
                        compile_error_ast(node,
                            "override of '%s' in '%s' has mismatched return type",
                            method_name, def->name);
                    }
                    free(existing->mangled_name);
                    existing->mangled_name = strdup(node->name);
                    existing->return_type = node->datatype;
                    existing->visibility = node->multiplier;
                    if (node->stack_index)
                        existing->is_virtual = 1;
                    existing->is_override = base_m ? 1 : 0;
                    free(method_name);
                    continue;
                }
                compile_error_ast(node, "duplicate method '%s' in cust '%s'",
                        method_name, def->name);
            }
            for (unsigned int j = 0; j < type->fields->size; j++) {
                cust_field_t *ef = (cust_field_t *)type->fields->items[j];
                if (ef && ef->name && strcmp(ef->name, method_name) == 0) {
                    compile_error_ast(node, "method '%s' conflicts with field in cust '%s'",
                            method_name, def->name);
                }
            }
            if (base_m && base_m->is_virtual && !node->stack_index) {
                compile_error_ast(node,
                    "method '%s' in '%s' hides virtual base method; use 'virtual' to override",
                    method_name, def->name);
            }
            if (node->stack_index && base_m && !base_m->is_virtual) {
                compile_error_ast(node,
                    "cannot override non-virtual method '%s' in '%s'",
                    method_name, def->name);
            }
            cust_method_t *method = calloc(1, sizeof(cust_method_t));
            method->name = method_name;
            method->mangled_name = strdup(node->name);
            method->return_type = node->datatype;
            method->visibility = node->multiplier;
            method->is_instance = node->id ? 1 : 0;
            method->is_virtual = node->stack_index ? 1 : 0;
            method->vtable_slot = -1;
            list_enqueue(type->methods, method);
            continue;
        }

        if (node->type != VAR_AST || !node->name) {
            compile_error_ast(def, "invalid member in cust '%s'", def->name);
        }
        if (!node->datatype || node->datatype == TYPE_UNKNOWN) {
            compile_error_ast(node, "unknown field type for '%s' in cust '%s'",
                    node->name, def->name);
        }
        if (IS_TYPE_PARAM(node->datatype)) {
            compile_error_ast(node, "unbound type parameter in field '%s'", node->name);
        }
        if (node->datatype == TYPE_ARRAY) {
            compile_error_ast(def, "Array fields need element type in cust '%s'", def->name);
        }
        if (cust_field_by_name(type->id, node->name)) {
            compile_error_ast(node, "duplicate field '%s' in cust '%s'", node->name, def->name);
        }
        if (method_in_type(type->id, node->name)) {
            compile_error_ast(node, "field '%s' conflicts with method in cust '%s'",
                    node->name, def->name);
        }

        int align = field_align(node->datatype);
        offset = align_up(offset, align);
        cust_field_t *field = calloc(1, sizeof(cust_field_t));
        field->name = strdup(node->name);
        field->datatype = node->datatype;
        field->offset = offset;
        field->visibility = node->multiplier;
        field->immutable = (node->int_value & AST_IMMPORTAL_FLAG) != 0;
        field->declaring_type_id = type->id;
        offset += field_size_for_datatype(node->datatype);
        list_enqueue(type->fields, field);
    }

    offset = align_up(offset, 8);
    type->size = offset;
    cust_build_vtable(type);
    if (!completing_forward)
        list_enqueue(cust_registry, type);

    if (implements_ifaces) {
        for (unsigned int i = 0; i < implements_ifaces->size; i++) {
            int iid = (int)(intptr_t)implements_ifaces->items[i];
            interface_check_implements(type->id, iid, def);
        }
    }
    return type->id;
}

char *cust_emit_vtables(void)
{
    if (!cust_registry || cust_registry->size == 0)
        return strdup("");

    size_t cap = 256;
    size_t len = 0;
    char *buf = calloc(cap, 1);
    if (!buf)
        return 0;

    for (unsigned int ti = 0; ti < cust_registry->size; ti++) {
        cust_type_t *t = (cust_type_t *)cust_registry->items[ti];
        if (!t || !t->has_vtable || t->vtable_slots <= 0)
            continue;

        char header[128];
        snprintf(header, sizeof(header), "%s__vtable:\n", t->name);
        len = strlen(buf);
        if (len + strlen(header) + 1 >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        strcat(buf, header);

        for (int slot = 0; slot < t->vtable_slots; slot++) {
            cust_method_t *chosen = 0;
            for (unsigned int mi = 0; mi < t->methods->size; mi++) {
                cust_method_t *m = (cust_method_t *)t->methods->items[mi];
                if (m && m->is_virtual && m->vtable_slot == slot) {
                    chosen = m;
                    break;
                }
            }
            if (!chosen) {
                compile_error_at(NULL, 0, "missing vtable slot %d for '%s'", slot, t->name);
            }
            char line[128];
            snprintf(line, sizeof(line), "    .quad %s\n", chosen->mangled_name);
            len = strlen(buf);
            if (len + strlen(line) + 1 >= cap) {
                cap *= 2;
                buf = realloc(buf, cap);
            }
            strcat(buf, line);
        }
    }
    return buf;
}

int cust_register_sum_type(const char *name, dynamic_list_t *enum_members, const AST_t *loc)
{
    if (!name || !enum_members) {
        compile_error_ast(loc, "invalid sum type definition");
    }
    if (cust_lookup_by_name(name) >= 0) {
        compile_error_ast(loc, "sum type '%s' already defined", name);
    }

    int payload_dt = TYPE_INT;
    int has_payload = 0;
    for (unsigned int i = 0; i < enum_members->size; i++) {
        AST_t *m = (AST_t *)enum_members->items[i];
        if (m && m->multiplier) {
            has_payload = 1;
            if (m->datatype != TYPE_INT && m->datatype != TYPE_UNKNOWN)
                compile_error_ast(loc, "sum type payloads currently support int only");
            payload_dt = TYPE_INT;
        }
    }
    if (!has_payload) {
        compile_error_ast(loc, "sum type '%s' requires at least one tagged variant", name);
    }

    dynamic_list_t *members = init_list(sizeof(struct AST_S *));
    AST_t *tag = calloc(1, sizeof(struct AST_S));
    tag->type = VAR_AST;
    tag->name = strdup("tag");
    tag->datatype = TYPE_INT;
    list_enqueue(members, tag);

    AST_t *payload = calloc(1, sizeof(struct AST_S));
    payload->type = VAR_AST;
    payload->name = strdup("payload");
    payload->datatype = payload_dt;
    list_enqueue(members, payload);

    AST_t *def = calloc(1, sizeof(struct AST_S));
    def->type = CUST_DEF_AST;
    def->name = strdup(name);
    def->children = members;
    if (loc) {
        def->source_line = loc->source_line;
        def->source_file = loc->source_file ? strdup(loc->source_file) : 0;
    }
    return cust_register_from_ast(def, -1, 0);
}

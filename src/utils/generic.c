#include "../include/generic.h"
#include "../include/numeric.h"
#include "../include/cust.h"
#include "../include/errors.h"
#include "../include/types.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *name;
    dynamic_list_t *param_names; /* char* */
    dynamic_list_t *members;     /* AST_t* shallow copy of member nodes */
    dynamic_list_t *iface_ids;   /* int as void* */
    int base_type_id;
} generic_template_t;

static dynamic_list_t *generic_templates;
static dynamic_list_t *instantiated_defs;

static AST_t *clone_ast_shallow(AST_t *node)
{
    if (!node)
        return 0;
    AST_t *copy = calloc(1, sizeof(struct AST_S));
    memcpy(copy, node, sizeof(struct AST_S));
    copy->children = 0;
    copy->left = 0;
    copy->right = 0;
    copy->parent = 0;
    copy->name = node->name ? strdup(node->name) : 0;
    copy->string_value = node->string_value ? strdup(node->string_value) : 0;
    copy->source_file = node->source_file ? strdup(node->source_file) : 0;
    if (node->children) {
        copy->children = init_list(sizeof(struct AST_S *));
        for (unsigned int i = 0; i < node->children->size; i++) {
            AST_t *child = (AST_t *)node->children->items[i];
            list_enqueue(copy->children, clone_ast_shallow(child));
        }
    }
    if (node->left)
        copy->left = clone_ast_shallow(node->left);
    if (node->right)
        copy->right = clone_ast_shallow(node->right);
    if (node->parent)
        copy->parent = clone_ast_shallow(node->parent);
    return copy;
}

static void free_template(generic_template_t *t)
{
    if (!t)
        return;
    free(t->name);
    if (t->param_names) {
        for (unsigned int i = 0; i < t->param_names->size; i++)
            free(t->param_names->items[i]);
        list_free(t->param_names);
    }
    if (t->members) {
        for (unsigned int i = 0; i < t->members->size; i++)
            ; /* members owned by template; freed with registry reset */
        list_free(t->members);
    }
    if (t->iface_ids)
        list_free_shallow(t->iface_ids);
    free(t);
}

void generic_registry_reset(void)
{
    if (!generic_templates && !instantiated_defs)
        return;
    if (generic_templates) {
        for (unsigned int i = 0; i < generic_templates->size; i++)
            free_template((generic_template_t *)generic_templates->items[i]);
        list_free(generic_templates);
        generic_templates = 0;
    }
    if (instantiated_defs) {
        list_free(instantiated_defs);
        instantiated_defs = 0;
    }
}

int generic_template_count(void)
{
    return generic_templates ? (int)generic_templates->size : 0;
}

int generic_lookup_template(const char *name)
{
    if (!name || !generic_templates)
        return -1;
    for (unsigned int i = 0; i < generic_templates->size; i++) {
        generic_template_t *t = (generic_template_t *)generic_templates->items[i];
        if (t && t->name && strcmp(t->name, name) == 0)
            return (int)i;
    }
    return -1;
}

void generic_register_template(const char *name, dynamic_list_t *param_names,
                               dynamic_list_t *member_nodes,
                               dynamic_list_t *iface_ids, int base_type_id,
                               const AST_t *loc)
{
    if (!name || !param_names || param_names->size == 0 || !member_nodes) {
        compile_error_ast(loc, "invalid generic type definition");
    }
    if (generic_lookup_template(name) >= 0) {
        compile_error_ast(loc, "generic type '%s' already defined", name);
    }
    if (cust_lookup_by_name(name) >= 0) {
        compile_error_ast(loc, "generic type '%s' conflicts with existing type name", name);
    }
    if (!generic_templates)
        generic_templates = init_list(sizeof(generic_template_t *));

    generic_template_t *t = calloc(1, sizeof(generic_template_t));
    t->name = strdup(name);
    t->param_names = init_list(sizeof(char *));
    for (unsigned int i = 0; i < param_names->size; i++)
        list_enqueue(t->param_names, strdup((char *)param_names->items[i]));

    t->members = init_list(sizeof(struct AST_S *));
    for (unsigned int i = 0; i < member_nodes->size; i++) {
        AST_t *m = (AST_t *)member_nodes->items[i];
        list_enqueue(t->members, clone_ast_shallow(m));
    }
    if (iface_ids && iface_ids->size > 0) {
        t->iface_ids = init_list(sizeof(void *));
        for (unsigned int i = 0; i < iface_ids->size; i++)
            list_enqueue(t->iface_ids, iface_ids->items[i]);
    }
    t->base_type_id = base_type_id;
    list_enqueue(generic_templates, t);
}

static int substitute_datatype(int dt, dynamic_list_t *param_dts)
{
    if (!IS_TYPE_PARAM(dt))
        return dt;
    int idx = TYPE_PARAM_INDEX(dt);
    if (idx < 0 || (unsigned int)idx >= param_dts->size)
        return TYPE_UNKNOWN;
    return (int)(intptr_t)param_dts->items[idx];
}

static void substitute_member_types(dynamic_list_t *members, dynamic_list_t *param_dts)
{
    for (unsigned int i = 0; i < members->size; i++) {
        AST_t *m = (AST_t *)members->items[i];
        if (!m)
            continue;
        if (m->datatype && m->datatype != TYPE_UNKNOWN)
            m->datatype = substitute_datatype(m->datatype, param_dts);
        if (m->type == FUNC_AST && m->parent)
            substitute_member_types(m->parent->children, param_dts);
    }
}

static void remangle_cust_methods(dynamic_list_t *members, const char *type_name)
{
    for (unsigned int i = 0; i < members->size; i++) {
        AST_t *m = (AST_t *)members->items[i];
        if (!m || m->type != FUNC_AST || !m->name)
            continue;
        char *short_name = NULL;
        if (strstr(m->name, "_operator_")) {
            char *p = strstr(m->name, "_operator_");
            short_name = strdup(p + 1);
        } else {
            char *underscore = strrchr(m->name, '_');
            if (!underscore || !underscore[1])
                continue;
            short_name = strdup(underscore + 1);
        }
        free(m->name);
        m->name = cust_mangle_method(type_name, short_name);
        free(short_name);
    }
}

static void append_dt_suffix(char *out, size_t cap, int dt)
{
    const char *suf = "unknown";
    if (dt == TYPE_INT)
        suf = "int";
    else if (IS_INT_TYPE(dt)) {
        snprintf(out, cap, "i%d", numeric_bit_width(dt));
        return;
    } else if (IS_FLOAT_TYPE(dt)) {
        snprintf(out, cap, "f%d", numeric_bit_width(dt));
        return;
    } else if (dt == TYPE_BOOL)
        suf = "bool";
    else if (dt == TYPE_STR)
        suf = "str";
    else if (dt == TYPE_ADR)
        suf = "adr";
    else if (IS_ARRAY_TYPE(dt))
        suf = "Array";
    else if (IS_CUST_TYPE(dt)) {
        cust_type_t *t = cust_get(CUST_TYPE_ID(dt));
        if (t && t->name)
            suf = t->name;
        else
            suf = "cust";
    }
    size_t used = strlen(out);
    if (used + 2 + strlen(suf) >= cap)
        return;
    if (used > 0)
        strcat(out, "_");
    strcat(out, suf);
}

static char *build_mangled_name(const char *base, dynamic_list_t *arg_dts)
{
    char out[256];
    out[0] = '\0';
    strcpy(out, base);
    for (unsigned int i = 0; i < arg_dts->size; i++) {
        int dt = (int)(intptr_t)arg_dts->items[i];
        append_dt_suffix(out, sizeof(out), dt);
    }
    return strdup(out);
}

int generic_instantiate(const char *name, dynamic_list_t *arg_dts, const AST_t *loc)
{
    int ti = generic_lookup_template(name);
    if (ti < 0) {
        compile_error_ast(loc, "unknown generic type '%s'", name);
    }
    generic_template_t *tmpl = (generic_template_t *)generic_templates->items[ti];
    if (arg_dts->size != tmpl->param_names->size) {
        compile_error_ast(loc, "generic '%s' expects %u type argument(s)",
                          name, (unsigned)tmpl->param_names->size);
    }

    char *mangled = build_mangled_name(name, arg_dts);
    int existing = cust_lookup_by_name(mangled);
    if (existing >= 0) {
        free(mangled);
        return existing;
    }

    dynamic_list_t *members = init_list(sizeof(struct AST_S *));
    for (unsigned int i = 0; i < tmpl->members->size; i++) {
        AST_t *m = clone_ast_shallow((AST_t *)tmpl->members->items[i]);
        list_enqueue(members, m);
    }
    substitute_member_types(members, arg_dts);
    remangle_cust_methods(members, mangled);

    AST_t *def = calloc(1, sizeof(struct AST_S));
    def->type = CUST_DEF_AST;
    def->name = mangled;
    def->children = members;
    if (loc) {
        def->source_line = loc->source_line;
        def->source_file = loc->source_file ? strdup(loc->source_file) : 0;
    }

    dynamic_list_t *ifaces = 0;
    if (tmpl->iface_ids && tmpl->iface_ids->size > 0) {
        ifaces = init_list(sizeof(void *));
        for (unsigned int i = 0; i < tmpl->iface_ids->size; i++)
            list_enqueue(ifaces, tmpl->iface_ids->items[i]);
    }
    int id = cust_register_from_ast(def, tmpl->base_type_id, ifaces);
    if (ifaces)
        list_free_shallow(ifaces);
    def->int_value = id;
    def->datatype = MAKE_CUST_TYPE(id);

    if (!instantiated_defs)
        instantiated_defs = init_list(sizeof(struct AST_S *));
    list_enqueue(instantiated_defs, def);

    return id;
}

void generic_append_instantiated_to_root(AST_t *root)
{
    if (!root || !root->children || !instantiated_defs || instantiated_defs->size == 0)
        return;

    size_t insert_after = 0;
    for (unsigned int i = 0; i < root->children->size; i++) {
        AST_t *child = (AST_t *)root->children->items[i];
        if (child && child->type == CUST_DEF_AST)
            insert_after = i + 1;
    }

    dynamic_list_t *merged = init_list(sizeof(struct AST_S *));
    for (unsigned int i = 0; i < root->children->size; i++) {
        if (i == insert_after) {
            for (unsigned int j = 0; j < instantiated_defs->size; j++)
                list_enqueue(merged, instantiated_defs->items[j]);
        }
        list_enqueue(merged, root->children->items[i]);
    }
    if (insert_after >= root->children->size) {
        for (unsigned int j = 0; j < instantiated_defs->size; j++)
            list_enqueue(merged, instantiated_defs->items[j]);
    }

    list_free_shallow(root->children);
    root->children = merged;
    list_free_shallow(instantiated_defs);
    instantiated_defs = 0;
}

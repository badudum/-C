#include "../include/module.h"
#include "../include/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *path;
    char *name;
} module_entry_t;

static dynamic_list_t *module_files;

static char *path_basename(const char *path)
{
    if (!path)
        return NULL;
    const char *base = path;
    for (const char *p = path; *p; p++) {
        if (*p == '/')
            base = p + 1;
    }
    return strdup(base);
}

static char *strip_minusc_ext(const char *base)
{
    if (!base)
        return NULL;
    size_t n = strlen(base);
    if (n > 7 && strcmp(base + n - 7, ".minusc") == 0) {
        char *out = calloc(n - 6, 1);
        if (out)
            memcpy(out, base, n - 7);
        return out;
    }
    return strdup(base);
}

static int path_suffix_match(const char *registered, const char *source)
{
    if (!registered || !source)
        return 0;
    if (strcmp(registered, source) == 0)
        return 1;
    size_t rl = strlen(registered);
    size_t sl = strlen(source);
    if (sl >= rl && strcmp(source + sl - rl, registered) == 0)
        return 1;
    const char *rb = strrchr(registered, '/');
    const char *sb = strrchr(source, '/');
    rb = rb ? rb + 1 : registered;
    sb = sb ? sb + 1 : source;
    return strcmp(rb, sb) == 0;
}

void module_registry_reset(void)
{
    if (!module_files)
        return;
    for (unsigned int i = 0; i < module_files->size; i++) {
        module_entry_t *e = (module_entry_t *)module_files->items[i];
        if (!e)
            continue;
        free(e->path);
        free(e->name);
        free(e);
    }
    list_free_shallow(module_files);
    module_files = 0;
}

void module_register_qualified(const char *full_path, const char *module_name)
{
    if (!full_path || !module_name || !module_name[0])
        return;
    if (!module_files)
        module_files = init_list(sizeof(module_entry_t *));
    module_entry_t *e = calloc(1, sizeof(module_entry_t));
    e->path = strdup(full_path);
    e->name = strdup(module_name);
    list_enqueue(module_files, e);
}

const char *module_name_for_source(const char *source_file)
{
    if (!source_file || !module_files)
        return NULL;
    for (unsigned int i = 0; i < module_files->size; i++) {
        module_entry_t *e = (module_entry_t *)module_files->items[i];
        if (e && path_suffix_match(e->path, source_file))
            return e->name;
    }
    return NULL;
}

int module_is_name(const char *name)
{
    if (!name || !module_files)
        return 0;
    for (unsigned int i = 0; i < module_files->size; i++) {
        module_entry_t *e = (module_entry_t *)module_files->items[i];
        if (e && e->name && strcmp(e->name, name) == 0)
            return 1;
    }
    return 0;
}

unsigned int module_registered_count(void)
{
    return module_files ? module_files->size : 0;
}

const char *module_registered_name(unsigned int index)
{
    if (!module_files || index >= module_files->size)
        return NULL;
    module_entry_t *e = (module_entry_t *)module_files->items[index];
    return e ? e->name : NULL;
}

char *module_mangle_symbol(const char *module_name, const char *symbol)
{
    if (!module_name || !symbol)
        return NULL;
    size_t len = strlen(module_name) + strlen(symbol) + 3;
    char *out = calloc(len, 1);
    if (!out)
        return NULL;
    snprintf(out, len, "%s__%s", module_name, symbol);
    return out;
}

char *module_name_from_ref_path(const char *ref_path)
{
    char *base = path_basename(ref_path);
    if (!base)
        return NULL;
    char *name = strip_minusc_ext(base);
    free(base);
    return name;
}

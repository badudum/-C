#include "include/preprocess.h"
#include "include/utils.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* Return directory part of path (newly allocated, caller frees). */
static char* dir_of(const char* path)
{
    size_t len = strlen(path);
    const char* last_slash = NULL;
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '/')
            last_slash = path + i;
    }
    if (last_slash == NULL)
        return mkstr(".");
    size_t dlen = (size_t)(last_slash - path);
    if (dlen == 0)
        return mkstr("/");
    char* dir = calloc(dlen + 1, 1);
    if (!dir) return NULL;
    memcpy(dir, path, dlen);
    dir[dlen] = '\0';
    return dir;
}

/* Resolve ref_path relative to base_dir. Returns new string (caller frees). */
static char* resolve_include_path(const char* base_dir, const char* ref_path)
{
    size_t bl = strlen(base_dir);
    size_t rl = strlen(ref_path);
    char* out = calloc(bl + rl + 3, 1);
    if (!out) return NULL;
    strcpy(out, base_dir);
    if (bl > 0 && base_dir[bl - 1] != '/')
        strcat(out, "/");
    strcat(out, ref_path);
    return out;
}

/* Return 1 if path is in the list (by strcmp). */
static int path_already_included(dynamic_list_t* list, const char* path)
{
    for (size_t i = 0; i < list->size; i++) {
        char* existing = (char*)list->items[i];
        if (existing && strcmp(existing, path) == 0)
            return 1;
    }
    return 0;
}

/* Parse one reference line: "reference \"path\"" or "reference <path>".
 * line should be trimmed of leading whitespace and start with "reference".
 * On success, set *path_start and *path_len to the path substring (within line), return 1.
 * Otherwise return 0. */
static int parse_reference_line(const char* line, const char** path_start, size_t* path_len)
{
    const char* p = line;
    while (*p && !isspace((unsigned char)*p))
        p++;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (*p == '"') {
        p++;
        *path_start = p;
        while (*p && *p != '"')
            p++;
        if (*p != '"')
            return 0;
        *path_len = (size_t)(p - *path_start);
        return 1;
    }
    if (*p == '<') {
        p++;
        *path_start = p;
        while (*p && *p != '>')
            p++;
        if (*p != '>')
            return 0;
        *path_len = (size_t)(p - *path_start);
        return 1;
    }
    return 0;
}

/* Test if line (after trimming leading space) is a reference directive. */
static int is_reference_line(const char* line)
{
    while (*line && isspace((unsigned char)*line))
        line++;
    if (strncmp(line, "reference", 8) != 0)
        return 0;
    line += 8;
    if (*line && !isspace((unsigned char)*line))
        return 0;
    return 1;
}

/* Copy substring to new null-terminated string. */
static char* path_from_substring(const char* start, size_t len)
{
    char* out = calloc(len + 1, 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

char* preprocess_source(const char* source, const char* current_file_path, dynamic_list_t* included_paths)
{
    char* base_dir = dir_of(current_file_path);
    if (!base_dir) return NULL;

    size_t cap = 4096;
    size_t used = 0;
    char* out = calloc(cap, 1);
    if (!out) { free(base_dir); return NULL; }

    const char* p = source;
    while (*p) {
        const char* line_start = p;
        while (*p && *p != '\n')
            p++;
        size_t line_len = (size_t)(p - line_start);
        if (*p == '\n')
            p++;

        /* Trim leading whitespace for check */
        const char* trim = line_start;
        while (line_len > 0 && isspace((unsigned char)*trim)) {
            trim++;
            line_len--;
        }

        if (line_len >= 8 && is_reference_line(trim)) {
            const char* path_start = NULL;
            size_t path_len = 0;
            if (parse_reference_line(trim, &path_start, &path_len)) {
                char* ref_path = path_from_substring(path_start, path_len);
                if (ref_path) {
                    char* full_path = resolve_include_path(base_dir, ref_path);
                    free(ref_path);
                    if (full_path) {
                        if (path_already_included(included_paths, full_path)) {
                            fprintf(stderr, "Preprocess: circular reference to '%s'\n", full_path);
                            free(full_path);
                            free(out);
                            free(base_dir);
                            return NULL;
                        }
                        char* file_src = read_file(full_path);
                        list_enqueue(included_paths, full_path);
                        char* expanded = preprocess_source(file_src, full_path, included_paths);
                        free(file_src);
                        /* Pop from list (list owns full_path) */
                        if (included_paths->size > 0) {
                            size_t last = included_paths->size - 1;
                            free(included_paths->items[last]);
                            included_paths->size--;
                        }
                        if (expanded) {
                            size_t elen = strlen(expanded);
                            while (used + elen + 2 >= cap) {
                                cap *= 2;
                                char* n = realloc(out, cap);
                                if (!n) { free(expanded); free(out); free(base_dir); return NULL; }
                                out = n;
                            }
                            memcpy(out + used, expanded, elen + 1);
                            used += elen;
                            if (used > 0 && out[used - 1] != '\n') {
                                out[used] = '\n';
                                used++;
                                out[used] = '\0';
                            }
                            free(expanded);
                        }
                    }
                }
            }
            /* Skip appending the reference line itself */
        } else {
            /* Append this line */
            size_t need = line_len + 2;
            while (used + need >= cap) {
                cap *= 2;
                char* n = realloc(out, cap);
                if (!n) { free(out); free(base_dir); return NULL; }
                out = n;
            }
            memcpy(out + used, line_start, line_len);
            used += line_len;
            if (*line_start || line_len > 0) {
                out[used] = '\n';
                used++;
            }
            out[used] = '\0';
        }
        if (*p == '\0')
            break;
    }

    free(base_dir);
    return out;
}

#include "include/preprocess.h"
#include "include/utils.h"
#include "include/errors.h"
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
    /* Unquoted path: rest of line (e.g. reference array_tests) */
    *path_start = p;
    while (*p && *p != '\r' && *p != '\n')
        p++;
    while (p > *path_start && isspace((unsigned char)p[-1]))
        p--;
    *path_len = (size_t)(p - *path_start);
    return *path_len > 0 ? 1 : 0;
}

#define REFERENCE_KW "reference"
#define REFERENCE_KW_LEN (sizeof(REFERENCE_KW) - 1)

/* Test if line (after trimming leading space) is a reference directive. */
static int is_reference_line(const char* line)
{
    while (*line && isspace((unsigned char)*line))
        line++;
    if (strncmp(line, REFERENCE_KW, REFERENCE_KW_LEN) != 0)
        return 0;
    line += REFERENCE_KW_LEN;
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

/* References always resolve to a .minusc file. Omit the extension in source; it is appended here. */
static char* ensure_minusc_suffix(const char* ref_path)
{
    static const char suf[] = ".minusc";
    size_t n = strlen(ref_path);
    if (n >= sizeof(suf) - 1 && strcmp(ref_path + n - (sizeof(suf) - 1), suf) == 0)
        return strdup(ref_path);
    char* out = calloc(n + sizeof(suf), 1);
    if (!out) return NULL;
    memcpy(out, ref_path, n);
    memcpy(out + n, suf, sizeof(suf));
    return out;
}

static void append_line_directive(char **out, size_t *used, size_t *cap, int line, const char *file)
{
    char buf[512];
    snprintf(buf, sizeof(buf), "#line %d \"%s\"\n", line, file ? file : "<unknown>");
    size_t blen = strlen(buf);
    while (*used + blen + 1 >= *cap) {
        *cap *= 2;
        char *n = realloc(*out, *cap);
        if (!n)
            return;
        *out = n;
    }
    memcpy(*out + *used, buf, blen);
    *used += blen;
    (*out)[*used] = '\0';
}

char* preprocess_source(const char* source, const char* current_file_path, dynamic_list_t* included_paths)
{
    char* base_dir = dir_of(current_file_path);
    if (!base_dir) return NULL;

    size_t cap = 4096;
    size_t used = 0;
    char* out = calloc(cap, 1);
    if (!out) { free(base_dir); return NULL; }

    append_line_directive(&out, &used, &cap, 1, current_file_path);
    int cur_line = 1;

    const char* p = source;
    while (*p) {
        const char* line_start = p;
        while (*p && *p != '\n')
            p++;
        size_t line_len = (size_t)(p - line_start);
        if (*p == '\n')
            p++;

        /* Trim leading whitespace only for reference detection; keep full line_len for copy */
        const char* trim = line_start;
        size_t trim_len = line_len;
        while (trim_len > 0 && isspace((unsigned char)*trim)) {
            trim++;
            trim_len--;
        }

        if (trim_len >= REFERENCE_KW_LEN && is_reference_line(trim)) {
            const char* path_start = NULL;
            size_t path_len = 0;
            if (parse_reference_line(trim, &path_start, &path_len)) {
                char* ref_path = path_from_substring(path_start, path_len);
                if (ref_path) {
                    char* ref_minusc = ensure_minusc_suffix(ref_path);
                    free(ref_path);
                    if (!ref_minusc) {
                        free(out);
                        free(base_dir);
                        return NULL;
                    }
                    char* full_path = resolve_include_path(base_dir, ref_minusc);
                    free(ref_minusc);
                    if (full_path) {
                        if (path_already_included(included_paths, full_path)) {
                            compile_error_at(current_file_path, cur_line,
                                             "Circular reference to '%s'", full_path);
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
            cur_line++;
            append_line_directive(&out, &used, &cap, cur_line, current_file_path);
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
            cur_line++;
        }
        if (*p == '\0')
            break;
    }

    free(base_dir);
    return out;
}

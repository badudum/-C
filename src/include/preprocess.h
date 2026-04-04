#ifndef PREPROCESS_H
#define PREPROCESS_H
#include "list.h"
#include <stdlib.h>

/* Expand all "reference <path>" and reference "path" lines in source.
 * current_file_path: path to the file this source came from (used for relative resolution).
 * included_paths: list of (char*) paths already being included (for cycle detection); will be updated.
 * Returns new allocated source string (caller frees). */
char* preprocess_source(const char* source, const char* current_file_path, dynamic_list_t* included_paths);

#endif

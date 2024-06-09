#ifndef UTILS_H
#define UTILS_H
#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void assert_not_null(void* ptr);

char* str_to_hex(const char* hex);

dynamic_list_t* str_to_hex_list(const char* hex);

char* mkstr(const char* str);

char* read_file(const char* filename);

int is_anagram(const char* str1, const char* str2);

#endif
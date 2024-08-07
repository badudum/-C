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

void write_file(const char* filename, const char* output_buffer);

char * command(const char* cmd);

int is_anagram(const char* str1, const char* str2);

char * str_format(char* instr);

char str_to_escape(const char*  instr);

#endif
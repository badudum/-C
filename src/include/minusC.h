#ifndef MINUSC_H
#define MINUSC_H
#include "utils.h"
#include "assembly.h"
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "visitor.h"

void compile(char * src, const char *filename);
void compile_set_check_only(int enabled);

void minusCompile_file(const char * filename);
void minusCheck_file(const char *filename);
#endif 
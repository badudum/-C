#ifndef MINUSC_H
#define MINUSC_H
#include "utils.h"
#include "lexer.h"
#include "parser.h"
#include "AST.h"
#include "visitor.h"

void compile(char * src);

void minusCompile_file(const char * filename);
#endif 
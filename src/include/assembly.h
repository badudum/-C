#ifndef ASSEMBLY_H
#define ASSEMBLY_H
#include "AST.h"


char * assemble(AST_t * ast, dynamic_list_t * list);

char * assemble_root(AST_t* ast, dynamic_list_t * list);

#endif
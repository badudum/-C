#ifndef VISITOR_H
#define VISITOR_H

#include "AST.h"
#include "list.h"
#include "stackframe.h"
#include "errors.h"
#include "utils.h"


typedef struct VISITOR_S
{
    AST_t* object;
}visitor_t;

AST_t* variable_lookup(dynamic_list_t* list, char* name);

visitor_t* init_visitor();

AST_t* visitor_visit(visitor_t* visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_compound(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_assignment(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_var(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_func(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_caller(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_int(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_str(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_binop(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_return(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

AST_t* visit_access(visitor_t * visitor, AST_t* node, dynamic_list_t* list, stackframe_t* stackframe);

#endif
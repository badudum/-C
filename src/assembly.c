
#include "include/assembly.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>

char * assemble_compound(AST_t* ast, dynamic_list_t* list)
{
    const char* template = "# compound (%p) \n";
    char * value = calloc(strlen(template) + 128, sizeof(char));
    sprintf(value, template, ast);

    for (unsigned int i = 0; i < ast->children->size; i ++)
    {
        AST_t* child = (AST_t*) ast->children->items[i];
        char * next = assemble(child, list);
        value = realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
        strcat(value, next);
    }
    return value;
}


char * assemble_assignment(AST_t * ast, dynamic_list_t * list)
{

}
char * assemble_variable(AST_t * ast, dynamic_list_t * list){}
char * assemble_call(AST_t * ast, dynamic_list_t * list){}
char * assemble_int(AST_t * ast, dynamic_list_t * list){}
char * assemble_string(AST_t * ast, dynamic_list_t * list){}
char * assemble_binop(AST_t * ast, dynamic_list_t * list){}
char * assemble_access(AST_t * ast, dynamic_list_t * list){}
char * assemble_return(AST_t * ast, dynamic_list_t * list){}


char * assemble_function(AST_t* ast, dynamic_list_t* list)
{
    // char * name = ast->name; 
    // int index = ast->stackframe->stack->size * 4;
// 
    // char * s = calloc((s))
}






char * assemble(AST_t * ast, dynamic_list_t * list)
{
    char * value = calloc(1, sizeof(char));

    char * next = 0;

    switch(ast->type)
    {
        case COMP_AST : next = assemble_compound(ast, list);break;
        case ASSIGNEMENT_AST : next = assemble_assignment(ast, list);break;
        case VAR_AST : next = assemble_variable(ast, list);break;
        case CALL_AST : next = assemble_call(ast, list);break;
        case INT_AST : next = assemble_int(ast, list); break;
        case STRING_AST : next = assemble_string(ast, list); break;
        case BINOP_AST : next = assemble_binop(ast, list); break;
        case ACCESS_AST : next = assemble_access(ast, list); break;
        case RETURN_AST : next = assemble_return(ast, list); break;
        case FUNC_AST : next = assemble_function(ast, list); break;
        default : {printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type); exit(1);} break;

    }

    value = realloc(value, (strlen(next) +1 ) * sizeof(char));
    strcat(value, next);

    return value;
}
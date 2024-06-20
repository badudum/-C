
#include "include/assembly.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "include/assembly/function_ass.h"
#include "include/assembly/assign_int_ass.h"
#include "include/assembly/assign_call_ass.h"
#include "include/assembly/assign_binop_ass.h"
#include "include/assembly/assign_default_ass.h"
#include "include/assembly/int_ass.h"
#include "include/assembly/root_ass.h"
#include "include/assembly/bootstrap_ass.h"
#include "include/assembly/access_ass.h"
#include "include/assembly/add_ass.h"
#include "include/assembly/sub_ass.h"
#include "include/assembly/mul_ass.h"
#include "include/assembly/div_ass.h"


/*
* The reason we allocate an additional 128 bytes is to ensure that we have enough space, and does not cause a buffer overflow
*/


char * assemble_compound(AST_t* ast, dynamic_list_t* list)
{
    const char* template = "\n # compound (%p) \n";
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


// this is the assembly conversion for assignment occurrences
char * assemble_assignment(AST_t * ast, dynamic_list_t * list)
{
    int id = (ast->stack_index * 8);

    char* s = calloc(1, sizeof(char));

    list_enqueue(list, ast);

    char* assemble_value = assemble(ast->parent , list);


    if(assemble_value)
    {
        s = realloc(s, (strlen(s) + strlen(assemble_value) + 1) * sizeof(char));
        strcat(s, assemble_value);
        free(assemble_value);
    }

    if(ast->datatype == TYPE_INT)
    {
        // what is mo? idk just mo with the flow
        char * mo = calloc(assembly_assignment_int_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_int_aarch64, id, ast->int_value);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }

    else if (ast->parent->type == CALL_AST)
    {
        char * mo = calloc(assembly_assignment_call_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_call_aarch64, id);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }

    else if(ast->parent->type == BINOP_AST)
    {
        char * mo = calloc(assembly_assignment_binop_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_binop_aarch64, id);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }

    else 
    {
        char * mo = calloc(assembly_assignment_default_aarch64_len + 128, sizeof(char));
        sprintf(mo, (char*) assembly_assignment_default_aarch64, id);
        s = realloc(s, (strlen(s) + strlen(mo) + 1) * sizeof(char));
        strcat(s, mo);
        free(mo);
    }
    return s;
}

// making a variable
char * assemble_variable(AST_t * ast, dynamic_list_t * list)
{
    char * s =  calloc(1, sizeof(char));

const char* template = "\n# variable (%s)\n"
                         "ldr x0, [fp, #%d]\n"
                         "str x0, [sp, #-16]!\n";

    int id = ((ast->stack_index)*8);
    s = realloc(s, (strlen(template) + 8) * sizeof(char));
    sprintf(s, template, ast->name, id);
    return s;
}

// making a function call
char * assemble_call(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    char * prefix = calloc(0, sizeof(char));

    bool has_prefix = false;

    unsigned int i = ast->parent->children->size;

    for(i = 0; i < ast->parent->children->size; i++)
    {
        AST_t* arg = (AST_t*) ast->parent->children->items[i];
        char * args = assemble(arg, list);

        s = realloc(s, (strlen(s) + strlen(args)+1) * sizeof(char));

        strcat(s, args);
    }

    for ( i = 0 ; i < ast->parent->children->size; i ++)
    {
        AST_t* arg = (AST_t*) ast->parent->children->items[i];

        const char* push_template = "\n# call arg\n"
                                    "ldr x0, [fp, #%d]\n"
                                    "str x0, [sp, #-16]!\n";

        char * push = calloc(strlen(push_template) + 128, sizeof(char));
        sprintf(push, push_template, (arg->stack_index + (arg->type == STRING_AST ? 1 : 0) * 8));
        s = realloc(s, (strlen(s) + strlen(push) + 1) * sizeof(char));
        strcat(s, push);
        free(push);
    }

    int add_size = i * 8;

    if(list->size)
    {
        if(((AST_t*)list->items[0])->type == ASSIGNEMENT_AST)
        {
            add_size = 0;
        }
    }



    const char* template = "bl %s\n"
                            "add sp, sp, #%d\n"
                            "str x0, [sp, #-16]!\n";

    char * ret = calloc(strlen(template) + 128, sizeof(char));
    sprintf(ret, template, ast->name, add_size);
    s = realloc(s, (strlen(s) + strlen(ret) + 1) * sizeof(char));
    strcat(s, ret);
    free(ret);

    char * f_string = calloc(strlen(s) + strlen(prefix) +1 , sizeof(char));
    strcat(f_string, prefix);
    strcat(f_string, s);

    free(s);

    if(has_prefix)
    {
        free(prefix);
    }

    return f_string;

}


char * assemble_int(AST_t * ast, dynamic_list_t * list)
{
    int index = ast->stack_index * 8;

    char * s = calloc(assembly_assignment_int_aarch64_len + 128, sizeof(char));
    sprintf(s, (char*) assembly_int_aarch64, ast->int_value, ast->int_value, index);

    return s;
}


char * assemble_string(AST_t * ast, dynamic_list_t * list)
{
    dynamic_list_t * chunks = str_to_hex_list(ast->string_value);
    unsigned int numb_bytes = ((chunks->size + 1) * 8);
    unsigned int byte_counter = numb_bytes - 8;

    int index = ast->stack_index * 8;

    const char* subl_template = "\n# %s\n"
                                "sub sp, sp, #%d\n";

    char * sub = calloc(strlen(subl_template) + 128, sizeof(char));
    sprintf(sub, subl_template, ast->string_value, numb_bytes);

    char * strpush = calloc(strlen(sub)+1, sizeof(char));
    strcat(strpush, sub);

    const char*  push_zero = "\nmov x0, #0\n"
                             "str x0, [sp, #%d]\n";
    
    char * zero = calloc(strlen(push_zero) + 128, sizeof(char));
    sprintf(zero, push_zero, byte_counter);

    strpush = realloc(strpush, (strlen(strpush) + strlen(zero) + 1) * sizeof(char));
    strcat(strpush, zero);

    byte_counter -= 8;  

    const char* pushtemplate = "ldr x0, =0x%s\n"
                           "str x0, [sp, #%d]\n";

    for(unsigned int i = 0; i < chunks->size ; i ++)
    {
        char * push_hex = (char*) chunks->items[(chunks->size - i)-1];
        char * push = calloc(strlen(pushtemplate) + strlen(push_hex) + 1, sizeof(char));
        sprintf(push, pushtemplate, push_hex, byte_counter);
        strpush = realloc(strpush, (strlen(strpush) + strlen(push) + 1) * sizeof(char));
        strcat(strpush, push);
        free(push);
        free(push_hex);
        byte_counter -= 8;
    }

    const char * final = "\n add x0, sp, #%d\n" // Adjusted to ARM64 syntax
                         "str x0, [fp, #%d]\n";
    
    char * fin = calloc(strlen(final) + 128, sizeof(char));
    sprintf(fin, final, 8, index+8);

    strpush = realloc(strpush, (strlen(strpush) + strlen(fin) + 1) * sizeof(char));

    strcat(strpush, fin);

    free(fin);

    return strpush;

}

/*
* This function is used to assemble the root of the AST
    Include Section Text:
        It starts by copying the section text from src_asm_root_asm into a buffer.
        This section text is defined in the root.h file and contains the initial assembly instructions.
    Process the AST:
        It then processes the AST by calling the as_f function, which generates the assembly code for the given AST node and its children.
        The generated code is appended to the buffer.
    Append Bootstrap Assembly Code:
        Finally, it appends the bootstrap assembly code from src_asm_bootstrap_asm to the buffer.
         This bootstrap code is necessary for setting up the initial state of the program.

*/
char * assemble_root(AST_t* ast, dynamic_list_t * list)
{
    const char * section = (char*) assemble_root_aarch64;
    char* value = calloc(assemble_root_aarch64_len + 128, sizeof(char));
    sprintf(value, section);

    char* next = assemble(ast, list);

    value = (char *)realloc(value, (strlen(value) + strlen(next) + 1) * sizeof(char));
    strcat(value, next);

    value = realloc(value, (strlen(value) + assembly_bootstrap_aarch64_len + 1) * sizeof(char));
    strcat(value, (char*) assembly_bootstrap_aarch64);

    return value;
}


char * assemble_binop(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    char * left_str = assemble(ast->left, list);
    char * right_str = assemble(ast->right, list);

    s = realloc(s, (strlen(left_str) + strlen(right_str) +1 ) * sizeof(char));

    strcat(s, right_str);
    strcat(s, left_str);
    

    char * value = 0;

    switch(ast->op)
    {
        case PLUS_TOKEN : value = (char*) assemble_add_aarch64; break;
        case MINUS_TOKEN : value = (char*) assemble_sub_aarch64; break;
        case ASTERISK_TOKEN : value = (char*) assemble_mul_aarch64; break;
        case SLASH_TOKEN : value = (char*) assemble_div_aarch64; break;
        default : {printf("ASSEMBLER: No front for AST of type `%d`\n", ast->type); exit(1);} break;
    }
}


char * assemble_access(AST_t * ast, dynamic_list_t * list)
{
    int offset = ((ast->stack_index * -1) * 8) - 8;
    int array_offset = MAX(8, (ast->int_value+1) *8);

    char * s = calloc(assemble_access_aarch64_len + 128, sizeof(char));
    sprintf(s, (char*) assemble_access_aarch64, offset, array_offset, array_offset, ast->stack_index * 8);

    return s;
}


char * assemble_return(AST_t * ast, dynamic_list_t * list)
{
    char * s = calloc(1, sizeof(char));

    const char* template = "\n%s\n"
                            "b return_statement\n";

    char * value = assemble(ast->parent, list);
    char * ret = calloc(strlen(template) + strlen(value) + 128, sizeof(char));
    sprintf(ret, template, value);

    s = realloc(s, (strlen(ret) + 1) * sizeof(char));
    strcat(s, ret);
    free(ret);

    return s;
}


char * assemble_function(AST_t* ast, dynamic_list_t* list)
{
    char * name = ast->name; 
    int index = ast->stackframe->stack->size * 8;

    char * s = calloc((assembly_function_begin_aarch64_len + (strlen(name)*2)+1), sizeof(char));

    sprintf(s, (char*) assembly_function_begin_aarch64, name, index);

    if(ast->stackframe->stack->size)
    {
        const char* sub_template = "\nsub sp, sp, #%d\n";
        char* sub = calloc(strlen(sub_template) + 128, sizeof(char));
        sprintf(sub, sub_template, (1+ ast->stackframe->stack->size) *8);

        s = realloc(s, (strlen(s)+ strlen(sub) +1) * sizeof(char));
        strcat(s, sub);
        free(sub);
    }
    AST_t* assembly_value = ast;

    for (unsigned int i = 0 ;i < assembly_value->children->size; i ++)
    {
        AST_t* function_arg = (AST_t*) assembly_value->children->items[i];
        AST_t* variable_arg = init_ast(VAR_AST);
        variable_arg->name = function_arg->name;
        variable_arg->int_value = (8* assembly_value->children->size) - ( i*8);
    }

    char* assemble_value_value = assemble(assembly_value->parent, list);
    s = realloc(s, (strlen(s) + strlen(assemble_value_value) +1) * sizeof(char));
    strcat(s, assemble_value_value);
    free(assemble_value_value);

    return s;
}






char * assemble(AST_t * ast, dynamic_list_t * list)
{
    // printf("Current : %d", ast->type);
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
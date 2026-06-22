#ifndef AST_H
#define AST_H
#include <stdlib.h>
#include "types.h"
#include "list.h"
#include "stackframe.h"
#include "token.h"

#define AST_IMMPORTAL_FLAG (1 << 20)
#define AST_NESTED_FUNC_LITERAL 2
#define CLOSURE_CAPTURE_MARK 10000

// struct for the abstract syntax tree
typedef struct AST_S
{
    enum 
    {   
        COMP_AST,
        FUNC_AST,
        CALL_AST,
        ASSIGNEMENT_AST = 99,
        DEF_TYPE_AST,
        VAR_AST,
        RETURN_AST,
        ACCESS_AST,
        INT_AST,
        FLOAT_AST,
        STRING_AST,
        BINOP_AST,
        SLICE_AST,
        ARRAY_LITERAL_AST,
        BOOL_AST,
        IF_AST,
        UNARY_AST,
        LOOP_UNTIL_AST,
        FOR_CLAUSE_AST,
        BREAK_AST,
        CONTINUE_AST,
        SWITCH_AST,
        CASE_AST,
        FOREACH_AST,
        ENUM_AST,
        INC_DEC_AST,
        DUPE_AST,
        TYPE_SIZE_AST,
        CUST_DEF_AST,
        CUST_INIT_AST,
        FIELD_ACCESS_AST,
        TRY_STMT_AST,
        NOOP_AST,
    }type;

    datatype datatype;

    dynamic_list_t * children;

    char * string_value;
    char * name;

    struct AST_S * left;
    struct AST_S * right;
    struct AST_S * parent;

    int op;
    int int_value;
    int id;
    int stack_index;
    int multiplier;

    struct AST_S * (*fptr)(struct VISITOR_S * visitor, struct AST_S * node, dynamic_list_t * list);
    stackframe_t * stackframe;

    int source_line;
    char *source_file;
} AST_t;

AST_t *  init_ast(int type);
void ast_set_loc_from_token(AST_t *ast, token_t *token);

#endif 
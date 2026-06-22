#include "../include/errors.h"
#include "../include/parser.h"
#include "../include/types.h"
#include "../include/numeric.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int g_json_diagnostics = 0;

void errors_set_json_diagnostics(int enabled)
{
    g_json_diagnostics = enabled ? 1 : 0;
}

static void vcompile_error_at(const char *file, int line, const char *fmt, va_list ap)
{
    if (g_json_diagnostics) {
        fprintf(stderr, "{\"file\":\"%s\",\"line\":%d,\"message\":\"",
                file && file[0] ? file : "", line > 0 ? line : 0);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\"}\n");
    } else if (file && line > 0) {
        fprintf(stderr, "Error at %s:%d: ", file, line);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
    } else {
        fprintf(stderr, "Error: ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
    }
    exit(1);
}

void compile_error_at(const char *file, int line, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vcompile_error_at(file, line, fmt, ap);
    va_end(ap);
}

void compile_error_ast(const AST_t *node, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (node && node->source_file && node->source_line > 0)
        vcompile_error_at(node->source_file, node->source_line, fmt, ap);
    else
        vcompile_error_at(NULL, 0, fmt, ap);
    va_end(ap);
}

void compile_error_parser(parser_t *parser, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (parser && parser->token && parser->token->source_file && parser->token->line > 0)
        vcompile_error_at(parser->token->source_file, parser->token->line, fmt, ap);
    else
        vcompile_error_at(NULL, 0, fmt, ap);
    va_end(ap);
}

static const char *datatype_name_for_error(int dt)
{
    if (IS_NUMERIC_TYPE(dt))
        return numeric_type_name(dt);
    if (IS_ARRAY_TYPE(dt))
        return "Array";
    if (IS_CUST_TYPE(dt))
        return "cust";
    switch (dt) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        case TYPE_ADR: return "adr";
        default: return "unknown";
    }
}

void compile_error_type_mismatch(const AST_t *node, int left_dt, int right_dt, int op_token)
{
    const char *lhs = datatype_name_for_error(left_dt);
    const char *rhs = datatype_name_for_error(right_dt);
    const char *hint = "";
    if (IS_INT_TYPE(left_dt) && IS_FLOAT_TYPE(right_dt))
        hint = " (hint: int promotes to float in mixed arithmetic)";
    else if (IS_FLOAT_TYPE(left_dt) && IS_INT_TYPE(right_dt))
        hint = " (hint: int promotes to float in mixed arithmetic)";
    else if (left_dt == TYPE_STR || right_dt == TYPE_STR)
        hint = " (hint: use + only for str concatenation)";
    (void)op_token;
    compile_error_ast(node, "invalid operands for operator on %s and %s%s", lhs, rhs, hint);
}

void check_arguments(AST_t* caller, AST_t* func)
{
    dynamic_list_t *params = (func && func->left && func->left->children)
        ? func->left->children
        : (func ? func->children : 0);
    int expected_args_num = params ? (int)params->size : 0;
    int actual_args_num = 0;
    if (caller->parent && caller->parent->children)
        actual_args_num = (int)caller->parent->children->size;

    if (expected_args_num != actual_args_num)
    {
        compile_error_ast(caller, "Expected %d arguments, but got %d",
                          expected_args_num, actual_args_num);
    }
}

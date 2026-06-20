#include "../include/errors.h"
#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void vcompile_error_at(const char *file, int line, const char *fmt, va_list ap)
{
    if (file && line > 0)
        fprintf(stderr, "Error at %s:%d: ", file, line);
    else
        fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
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

void check_arguments(AST_t* caller, AST_t* func)
{
    int expected_args_num = func->children->size;
    int actual_args_num = 0;
    if (caller->parent && caller->parent->children)
        actual_args_num = (int)caller->parent->children->size;

    if (expected_args_num != actual_args_num)
    {
        compile_error_ast(caller, "Expected %d arguments, but got %d",
                          expected_args_num, actual_args_num);
    }
}

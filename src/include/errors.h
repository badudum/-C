#ifndef ERRORS_H
#define ERRORS_H
#include "AST.h"

struct PARSER_S;
typedef struct PARSER_S parser_t;

void check_arguments(AST_t* caller, AST_t* func);

void compile_error_at(const char *file, int line, const char *fmt, ...);
void compile_error_ast(const AST_t *node, const char *fmt, ...);
void compile_error_parser(parser_t *parser, const char *fmt, ...);
void compile_error_type_mismatch(const AST_t *node, int left_dt, int right_dt, int op_token);
void errors_set_json_diagnostics(int enabled);

#endif

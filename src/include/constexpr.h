#ifndef CONSTEXPR_H
#define CONSTEXPR_H
#include "AST.h"
#include "list.h"

/* If node is a compile-time int expression, set *out and return 1. */
int constexpr_eval_int(AST_t *node, dynamic_list_t *list, int *out);

#endif

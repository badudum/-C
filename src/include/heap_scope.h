#ifndef HEAP_SCOPE_H
#define HEAP_SCOPE_H

#include "visitor.h"
#include "list.h"
#include "stackframe.h"

void heap_scope_init(visitor_t *visitor);
void heap_scope_free(visitor_t *visitor);

void heap_scope_push(visitor_t *visitor);
void heap_scope_pop_emit(visitor_t *visitor, AST_t *compound, dynamic_list_t *list,
                          stackframe_t *stackframe);

void heap_scope_register(visitor_t *visitor, const char *name, int cust_id,
                         int stack_index, stackframe_t *stackframe);
void heap_scope_mark_dropped(visitor_t *visitor, const char *name);

#endif

#ifndef NUMERIC_H
#define NUMERIC_H

#include "types.h"
#include "token.h"

#define NUMERIC_MAX_DOUBLE_PREFIX 4

#define IS_INT_TYPE(dt) ((dt) == TYPE_INT || ((dt) >= TYPE_I64 && (dt) <= TYPE_I256) || (dt) == TYPE_I512)
#define IS_FLOAT_TYPE(dt) ((dt) >= TYPE_F64 && (dt) <= TYPE_F1024)
#define IS_NUMERIC_TYPE(dt) (IS_INT_TYPE(dt) || IS_FLOAT_TYPE(dt))

int wint_from_double_count(int doubles);
int wfloat_from_double_count(int doubles);
int numeric_int_level(int dt);
int numeric_float_level(int dt);
int numeric_bit_width(int dt);
int numeric_byte_size(int dt);
int numeric_stack_slots(int dt);

/* Java-like binary promotion for + - * / and comparisons (not bitwise). */
int numeric_promote_binary(int left_dt, int right_dt, int op);

const char *numeric_type_name(int dt);
const char *numeric_runtime_binop_name(int op);
const char *numeric_runtime_cmp_name(int op);

#endif

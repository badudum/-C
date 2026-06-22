#include "../include/numeric.h"
#include <stdio.h>
#include <string.h>

static int wint_level_types[] = { TYPE_INT, TYPE_I64, TYPE_I128, TYPE_I256, TYPE_I512 };
static int wfloat_level_types[] = { TYPE_F64, TYPE_F128, TYPE_F256, TYPE_F512, TYPE_F1024 };

int wint_from_double_count(int doubles)
{
    if (doubles < 0 || doubles > NUMERIC_MAX_DOUBLE_PREFIX)
        return TYPE_UNKNOWN;
    return wint_level_types[doubles];
}

int wfloat_from_double_count(int doubles)
{
    if (doubles < 0 || doubles > NUMERIC_MAX_DOUBLE_PREFIX)
        return TYPE_UNKNOWN;
    return wfloat_level_types[doubles];
}

int numeric_int_level(int dt)
{
    for (int i = 0; i <= NUMERIC_MAX_DOUBLE_PREFIX; i++) {
        if (wint_level_types[i] == dt)
            return i;
    }
    return -1;
}

int numeric_float_level(int dt)
{
    for (int i = 0; i <= NUMERIC_MAX_DOUBLE_PREFIX; i++) {
        if (wfloat_level_types[i] == dt)
            return i;
    }
    return -1;
}

int numeric_bit_width(int dt)
{
    int il = numeric_int_level(dt);
    if (il >= 0)
        return 32 << il;
    int fl = numeric_float_level(dt);
    if (fl >= 0)
        return 64 << fl;
    return 0;
}

int numeric_byte_size(int dt)
{
    int bits = numeric_bit_width(dt);
    if (bits <= 0)
        return 0;
    return (bits + 7) / 8;
}

int numeric_stack_slots(int dt)
{
    int bytes = numeric_byte_size(dt);
    if (bytes <= 0)
        return 1;
    if (bytes <= 16)
        return 1;
    return (bytes + 15) / 16;
}

static int normalize_to_int(int dt)
{
    if (dt == TYPE_BOOL)
        return TYPE_INT;
    return dt;
}

static int max_int_type(int a, int b)
{
    int la = numeric_int_level(a);
    int lb = numeric_int_level(b);
    if (la < 0 || lb < 0)
        return TYPE_UNKNOWN;
    return wint_level_types[la > lb ? la : lb];
}

static int max_float_type(int a, int b)
{
    int la = numeric_float_level(a);
    int lb = numeric_float_level(b);
    if (la < 0 || lb < 0)
        return TYPE_UNKNOWN;
    return wfloat_level_types[la > lb ? la : lb];
}

int numeric_promote_binary(int left_dt, int right_dt, int op)
{
    left_dt = normalize_to_int(left_dt);
    right_dt = normalize_to_int(right_dt);

    if (op == BITAND_TOKEN || op == BITOR_TOKEN) {
        if (!IS_INT_TYPE(left_dt) || !IS_INT_TYPE(right_dt))
            return TYPE_UNKNOWN;
        return max_int_type(left_dt, right_dt);
    }

    if (IS_FLOAT_TYPE(left_dt) || IS_FLOAT_TYPE(right_dt)) {
        int lf = IS_FLOAT_TYPE(left_dt) ? numeric_float_level(left_dt) : 0;
        int rf = IS_FLOAT_TYPE(right_dt) ? numeric_float_level(right_dt) : 0;
        /* Java-like: int + float promotes to at least 64-bit float; wider float wins. */
        if (!IS_FLOAT_TYPE(left_dt))
            lf = 0;
        if (!IS_FLOAT_TYPE(right_dt))
            rf = 0;
        return wfloat_level_types[lf > rf ? lf : rf];
    }

    if (IS_INT_TYPE(left_dt) && IS_INT_TYPE(right_dt))
        return max_int_type(left_dt, right_dt);

    return TYPE_UNKNOWN;
}

const char *numeric_type_name(int dt)
{
    static char buf[64];
    int il = numeric_int_level(dt);
    if (il == 0)
        return "int";
    if (il > 0) {
        snprintf(buf, sizeof(buf), "%d-bit int", numeric_bit_width(dt));
        return buf;
    }
    int fl = numeric_float_level(dt);
    if (fl >= 0) {
        snprintf(buf, sizeof(buf), "%d-bit float", numeric_bit_width(dt));
        return buf;
    }
    return "numeric";
}

const char *numeric_runtime_binop_name(int op)
{
    switch (op) {
        case PLUS_TOKEN: return "NumAdd";
        case MINUS_TOKEN: return "NumSub";
        case ASTERISK_TOKEN: return "NumMul";
        case SLASH_TOKEN: return "NumDiv";
        case MODULUS_TOKEN: return "NumMod";
        case CARET_TOKEN: return "NumPow";
        default: return "NumAdd";
    }
}

const char *numeric_runtime_cmp_name(int op)
{
    switch (op) {
        case DEQUALS_TOKEN: return "NumEq";
        case NOT_EQUALS_TOKEN: return "NumNe";
        case LT_TOKEN: return "NumLt";
        case GT_TOKEN: return "NumGt";
        case LTE_TOKEN: return "NumLe";
        case GTE_TOKEN: return "NumGe";
        default: return "NumEq";
    }
}

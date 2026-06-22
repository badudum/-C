#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MC_MAX_LIMBS 16
#define MC_PRINT_BUF 2048

static char mc_print_buf[MC_PRINT_BUF];

static int limb_count(int bits)
{
    return (bits + 63) / 64;
}

static void limbs_zero(uint64_t *v, int n)
{
    memset(v, 0, (size_t)n * sizeof(uint64_t));
}

static void limbs_copy(uint64_t *dst, const uint64_t *src, int n)
{
    memcpy(dst, src, (size_t)n * sizeof(uint64_t));
}

static void limbs_sub_inplace(uint64_t *a, const uint64_t *b, int n);
static void limbs_add_into(uint64_t *dst, const uint64_t *a, const uint64_t *b, int n);
static void limbs_udivmod(uint64_t *q, uint64_t *r, const uint64_t *u, const uint64_t *v, int n);

static int limbs_is_zero(const uint64_t *v, int n)
{
    for (int i = 0; i < n; i++)
        if (v[i] != 0)
            return 0;
    return 1;
}

static void limbs_mul(uint64_t *dst, const uint64_t *a, const uint64_t *b, int n)
{
    uint64_t prod[MC_MAX_LIMBS * 2];
    limbs_zero(prod, n * 2);

    for (int i = 0; i < n; i++) {
        unsigned __int128 carry = 0;
        for (int j = 0; j < n; j++) {
            unsigned __int128 cur = (unsigned __int128)prod[i + j] +
                                    (unsigned __int128)a[i] * (unsigned __int128)b[j] + carry;
            prod[i + j] = (uint64_t)cur;
            carry = cur >> 64;
        }
        prod[i + n] = (uint64_t)carry;
    }

    for (int i = 0; i < n; i++)
        dst[i] = prod[i];
}

static uint32_t limbs_div10(uint64_t *v, int n)
{
    unsigned __int128 rem = 0;
    for (int i = n - 1; i >= 0; i--) {
        unsigned __int128 cur = (rem << 64) | (unsigned __int128)v[i];
        v[i] = (uint64_t)(cur / 10);
        rem = cur % 10;
    }
    return (uint32_t)rem;
}

const char *mc_wide_print_bits(int bits, const uint64_t *src)
{
    int n = limb_count(bits);
    if (n <= 0 || n > MC_MAX_LIMBS)
        return "0";

    uint64_t work[MC_MAX_LIMBS];
    limbs_copy(work, src, n);

    char *out = mc_print_buf + MC_PRINT_BUF - 1;
    *out = '\0';

    if (limbs_is_zero(work, n)) {
        out--;
        *out = '0';
        return out;
    }

    while (!limbs_is_zero(work, n)) {
        uint32_t digit = limbs_div10(work, n);
        out--;
        *out = (char)('0' + digit);
    }
    return out;
}

void mc_wide_pow_bits(int bits, uint64_t *dst, const uint64_t *base, uint64_t exp)
{
    int n = limb_count(bits);
    if (n <= 0 || n > MC_MAX_LIMBS) {
        if (n > 0)
            limbs_zero(dst, n);
        return;
    }

    uint64_t result[MC_MAX_LIMBS];
    uint64_t cur_base[MC_MAX_LIMBS];
    uint64_t tmp[MC_MAX_LIMBS];

    limbs_zero(result, n);
    result[0] = 1;
    limbs_copy(cur_base, base, n);

    while (exp > 0) {
        if (exp & 1) {
            limbs_mul(tmp, result, cur_base, n);
            limbs_copy(result, tmp, n);
        }
        limbs_mul(tmp, cur_base, cur_base, n);
        limbs_copy(cur_base, tmp, n);
        exp >>= 1;
    }

    limbs_copy(dst, result, n);
}

const char *mc_ftos(double v)
{
    static char buf[64];
    snprintf(buf, sizeof(buf), "%.17g", v);
    return buf;
}

static int float_upper_limbs_zero(const uint64_t *v, int n)
{
    for (int i = 1; i < n; i++)
        if (v[i] != 0)
            return 0;
    return 1;
}

const char *mc_wide_float_print_bits(int bits, const uint64_t *src)
{
    int n = limb_count(bits);
    if (n <= 0 || n > MC_MAX_LIMBS)
        return "0";
    if (float_upper_limbs_zero(src, n)) {
        double v;
        memcpy(&v, src, sizeof(double));
        return mc_ftos(v);
    }
    return mc_wide_print_bits(bits, src);
}

void mc_wide_float_binop_bits(int op, int bits, uint64_t *dst,
                              const uint64_t *a, const uint64_t *b)
{
    int n = limb_count(bits);
    if (n <= 0 || n > MC_MAX_LIMBS) {
        if (n > 0)
            limbs_zero(dst, n);
        return;
    }

    if (float_upper_limbs_zero(a, n) && float_upper_limbs_zero(b, n)) {
        double da, db, dr;
        memcpy(&da, a, sizeof(double));
        memcpy(&db, b, sizeof(double));
        switch (op) {
            case 0: dr = da + db; break;
            case 1: dr = da - db; break;
            case 2: dr = da * db; break;
            case 3: dr = (db != 0.0) ? da / db : 0.0; break;
            default: dr = da + db; break;
        }
        limbs_zero(dst, n);
        memcpy(dst, &dr, sizeof(double));
        return;
    }

    if (op == 0) {
        limbs_add_into(dst, a, b, n);
        return;
    }
    if (op == 1) {
        limbs_copy(dst, a, n);
        limbs_sub_inplace(dst, b, n);
        return;
    }
    if (op == 2) {
        limbs_mul(dst, a, b, n);
        return;
    }
    if (op == 3) {
        if (limbs_is_zero(b, n)) {
            limbs_zero(dst, n);
            return;
        }
        uint64_t q[MC_MAX_LIMBS];
        uint64_t r[MC_MAX_LIMBS];
        limbs_udivmod(q, r, a, b, n);
        limbs_copy(dst, q, n);
        return;
    }
}

static int limbs_cmp_u(const uint64_t *a, const uint64_t *b, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        if (a[i] > b[i])
            return 1;
        if (a[i] < b[i])
            return -1;
    }
    return 0;
}

static void limbs_sub_inplace(uint64_t *a, const uint64_t *b, int n)
{
    unsigned __int128 borrow = 0;
    for (int i = 0; i < n; i++) {
        unsigned __int128 diff = (unsigned __int128)a[i] - b[i] - borrow;
        a[i] = (uint64_t)diff;
        borrow = (diff >> 64) & 1;
    }
}

static void limbs_add_into(uint64_t *dst, const uint64_t *a, const uint64_t *b, int n)
{
    unsigned __int128 carry = 0;
    for (int i = 0; i < n; i++) {
        unsigned __int128 sum = (unsigned __int128)a[i] + b[i] + carry;
        dst[i] = (uint64_t)sum;
        carry = sum >> 64;
    }
}

static void limbs_udivmod(uint64_t *q, uint64_t *r, const uint64_t *u, const uint64_t *v, int n)
{
    limbs_zero(q, n);
    limbs_copy(r, u, n);

    for (int i = n * 64 - 1; i >= 0; i--) {
        for (int j = n - 1; j > 0; j--)
            r[j] = (r[j] << 1) | (r[j - 1] >> 63);
        r[0] <<= 1;

        int limb = i / 64;
        int bit = i % 64;
        if ((u[limb] >> bit) & 1)
            r[0] |= 1;

        if (limbs_cmp_u(r, v, n) >= 0) {
            limbs_sub_inplace(r, v, n);
            q[limb] |= ((uint64_t)1 << bit);
        }
    }
}

static int limbs_upper_zero(const uint64_t *v, int n)
{
    for (int i = 1; i < n; i++)
        if (v[i] != 0)
            return 0;
    return 1;
}

void mc_wide_int_binop_bits(int op, int bits, uint64_t *dst,
                            const uint64_t *a, const uint64_t *b)
{
    int n = limb_count(bits);
    if (n <= 0 || n > MC_MAX_LIMBS) {
        if (n > 0)
            limbs_zero(dst, n);
        return;
    }

    uint64_t tmp[MC_MAX_LIMBS];
    uint64_t q[MC_MAX_LIMBS];
    uint64_t r[MC_MAX_LIMBS];

    if (op >= 2 && op <= 4 && limbs_upper_zero(a, n) && limbs_upper_zero(b, n) &&
        !limbs_is_zero(b, n)) {
        uint64_t va = a[0];
        uint64_t vb = b[0];
        limbs_zero(dst, n);
        if (op == 2)
            dst[0] = va * vb;
        else if (op == 3)
            dst[0] = va / vb;
        else
            dst[0] = va % vb;
        return;
    }

    switch (op) {
        case 0:
            limbs_add_into(dst, a, b, n);
            break;
        case 1:
            limbs_copy(tmp, a, n);
            limbs_sub_inplace(tmp, b, n);
            limbs_copy(dst, tmp, n);
            break;
        case 2:
            limbs_mul(dst, a, b, n);
            break;
        case 3:
            if (limbs_is_zero(b, n)) {
                limbs_zero(dst, n);
                break;
            }
            limbs_udivmod(q, r, a, b, n);
            limbs_copy(dst, q, n);
            break;
        case 4:
            if (limbs_is_zero(b, n)) {
                limbs_zero(dst, n);
                break;
            }
            limbs_udivmod(q, r, a, b, n);
            limbs_copy(dst, r, n);
            break;
        default:
            limbs_copy(dst, a, n);
            break;
    }
}

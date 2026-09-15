/* IEEE 754 soft-float for 64–1024 bit widths (included from numeric_rt.c). */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifndef MC_MAX_LIMBS
#define MC_MAX_LIMBS 16
#endif

typedef struct {
    int sign;
    int64_t exp;
    int cls;
#define SF_ZERO 1
#define SF_NORMAL 2
#define SF_INF 3
#define SF_NAN 4
    int mant_limbs;
    uint64_t mant[MC_MAX_LIMBS * 2];
} sf_t;

static int sf_exp_bits(int bits)
{
    switch (bits) {
        case 64: return 11;
        case 128: return 15;
        case 256: return 19;
        case 512: return 23;
        case 1024: return 27;
        default: return 11;
    }
}

static int sf_mant_bits(int bits)
{
    return bits - 1 - sf_exp_bits(bits);
}

static int sf_exp_bias(int bits)
{
    int eb = sf_exp_bits(bits);
    return (1 << (eb - 1)) - 1;
}

static int sf_limb_count(int bits)
{
    return (bits + 63) / 64;
}

static int sf_upper_limbs_zero(const uint64_t *v, int n)
{
    for (int i = 1; i < n; i++)
        if (v[i] != 0)
            return 0;
    return 1;
}

static void sf_zero_limbs(uint64_t *v, int n)
{
    memset(v, 0, (size_t)n * sizeof(uint64_t));
}

static int sf_is_zero_limbs(const uint64_t *v, int n)
{
    for (int i = 0; i < n; i++)
        if (v[i])
            return 0;
    return 1;
}

static int sf_msb_index(const uint64_t *v, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        if (!v[i])
            continue;
        for (int b = 63; b >= 0; b--)
            if ((v[i] >> b) & 1)
                return i * 64 + b;
    }
    return -1;
}

static void sf_shl_limbs(uint64_t *v, int n, int s)
{
    if (s <= 0)
        return;
    if (s >= n * 64) {
        sf_zero_limbs(v, n);
        return;
    }
    int ws = s / 64;
    int bs = s % 64;
    for (int i = n - 1; i >= 0; i--) {
        uint64_t val = 0;
        if (i - ws >= 0)
            val = v[i - ws] << bs;
        if (bs && i - ws - 1 >= 0)
            val |= v[i - ws - 1] >> (64 - bs);
        v[i] = val;
    }
    for (int i = 0; i < ws; i++)
        v[i] = 0;
}

static void sf_shr_limbs(uint64_t *v, int n, int s)
{
    if (s <= 0)
        return;
    if (s >= n * 64) {
        sf_zero_limbs(v, n);
        return;
    }
    int ws = s / 64;
    int bs = s % 64;
    for (int i = 0; i < n; i++) {
        uint64_t val = 0;
        if (i + ws < n)
            val = v[i + ws] >> bs;
        if (bs && i + ws + 1 < n)
            val |= v[i + ws + 1] << (64 - bs);
        v[i] = val;
    }
    for (int i = n - ws; i < n; i++)
        if (i >= 0)
            v[i] = 0;
}

static int sf_cmp_limbs(const uint64_t *a, const uint64_t *b, int n)
{
    for (int i = n - 1; i >= 0; i--) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

static void sf_add_limbs(uint64_t *r, const uint64_t *a, const uint64_t *b, int n)
{
    unsigned __int128 c = 0;
    for (int i = 0; i < n; i++) {
        unsigned __int128 s = (unsigned __int128)a[i] + b[i] + c;
        r[i] = (uint64_t)s;
        c = s >> 64;
    }
}

static void sf_sub_limbs(uint64_t *r, const uint64_t *a, const uint64_t *b, int n)
{
    unsigned __int128 br = 0;
    for (int i = 0; i < n; i++) {
        unsigned __int128 d = (unsigned __int128)a[i] - b[i] - br;
        r[i] = (uint64_t)d;
        br = (d >> 64) & 1;
    }
}

static void sf_mul_limbs(uint64_t *dst, const uint64_t *a, const uint64_t *b, int na, int nb)
{
    int nd = na + nb;
    if (nd > MC_MAX_LIMBS * 2)
        nd = MC_MAX_LIMBS * 2;
    sf_zero_limbs(dst, nd);
    for (int i = 0; i < na && i < MC_MAX_LIMBS; i++) {
        unsigned __int128 c = 0;
        for (int j = 0; j < nb && i + j < nd; j++) {
            unsigned __int128 cur = (unsigned __int128)dst[i + j] +
                                    (unsigned __int128)a[i] * b[j] + c;
            dst[i + j] = (uint64_t)cur;
            c = cur >> 64;
        }
        if (i + nb < nd)
            dst[i + nb] = (uint64_t)c;
    }
}

static int sf_get_bit(const uint64_t *v, int n, int bi)
{
    if (bi < 0 || bi >= n * 64)
        return 0;
    return (int)((v[bi / 64] >> (bi % 64)) & 1u);
}

static void sf_set_bit(uint64_t *v, int n, int bi, int on)
{
    if (bi < 0 || bi >= n * 64)
        return;
    uint64_t m = (uint64_t)1 << (bi % 64);
    if (on)
        v[bi / 64] |= m;
    else
        v[bi / 64] &= ~m;
}

static void sf_from_double(double d, sf_t *o, int bits)
{
    uint64_t u;
    memcpy(&u, &d, sizeof(u));
    memset(o, 0, sizeof(*o));
    o->mant_limbs = (sf_mant_bits(bits) + 70) / 64;
    if (o->mant_limbs < 2)
        o->mant_limbs = 2;
    o->sign = (int)(u >> 63);
    uint64_t e = (u >> 52) & 0x7FF;
    uint64_t f = u & 0xFFFFFFFFFFFFFULL;
    if (e == 0x7FF) {
        o->cls = f ? SF_NAN : SF_INF;
        return;
    }
    if (e == 0 && f == 0) {
        o->cls = SF_ZERO;
        return;
    }
    o->cls = SF_NORMAL;
    if (e == 0) {
        o->exp = 1 - sf_exp_bias(bits);
        o->mant[0] = f;
        while ((o->mant[0] & 1) == 0 && sf_is_zero_limbs(o->mant + 1, o->mant_limbs - 1)) {
            o->mant[0] >>= 1;
            o->exp--;
        }
        return;
    }
    o->exp = (int64_t)e - 1023;
    o->mant[0] = (1ULL << 52) | f;
}

static int sf_to_double(const sf_t *in, double *out)
{
    if (in->cls == SF_NAN) {
        *out = NAN;
        return 1;
    }
    if (in->cls == SF_INF) {
        *out = in->sign ? -INFINITY : INFINITY;
        return 1;
    }
    if (in->cls == SF_ZERO) {
        *out = in->sign ? -0.0 : 0.0;
        return 1;
    }
    sf_t t = *in;
    int mb = 52;
    int msb = sf_msb_index(t.mant, t.mant_limbs);
    if (msb > mb) {
        int sh = msb - mb;
        sf_shr_limbs(t.mant, t.mant_limbs, sh);
        t.exp += sh;
    } else if (msb >= 0 && msb < mb) {
        sf_shl_limbs(t.mant, t.mant_limbs, mb - msb);
        t.exp -= (mb - msb);
    }
    if (t.exp > 1023) {
        *out = t.sign ? -INFINITY : INFINITY;
        return 1;
    }
    if (t.exp < -1022) {
        *out = t.sign ? -0.0 : 0.0;
        return 1;
    }
    uint64_t u = 0;
    if (t.sign) u |= 1ULL << 63;
    u |= ((uint64_t)(t.exp + 1023) & 0x7FF) << 52;
    u |= t.mant[0] & 0xFFFFFFFFFFFFFULL;
    memcpy(out, &u, sizeof(*out));
    return 1;
}

static void sf_decode(const uint64_t *src, int bits, sf_t *o)
{
    int n = sf_limb_count(bits);
    if (bits == 64 || sf_upper_limbs_zero(src, n)) {
        double d;
        memcpy(&d, src, sizeof(double));
        sf_from_double(d, o, bits);
        return;
    }

    int eb = sf_exp_bits(bits);
    int mb = sf_mant_bits(bits);
    int bias = sf_exp_bias(bits);
    memset(o, 0, sizeof(*o));
    o->mant_limbs = (mb + 70) / 64;

    o->sign = sf_get_bit(src, n, bits - 1);
    uint64_t efield = 0;
    for (int i = 0; i < eb; i++)
        if (sf_get_bit(src, n, bits - 2 - i))
            efield |= (uint64_t)1 << (eb - 1 - i);

    for (int i = 0; i < mb; i++)
        if (sf_get_bit(src, n, i))
            o->mant[i / 64] |= (uint64_t)1 << (i % 64);

    if (efield == (uint64_t)((1u << eb) - 1)) {
        o->cls = sf_is_zero_limbs(o->mant, o->mant_limbs) ? SF_INF : SF_NAN;
        return;
    }
    if (efield == 0) {
        if (sf_is_zero_limbs(o->mant, o->mant_limbs)) {
            o->cls = SF_ZERO;
            return;
        }
        o->cls = SF_NORMAL;
        o->exp = 1 - bias;
        return;
    }
    o->cls = SF_NORMAL;
    o->exp = (int64_t)efield - bias;
    if (mb < 64)
        o->mant[0] |= (uint64_t)1 << mb;
    else
        o->mant[mb / 64] |= (uint64_t)1 << (mb % 64);
}

static void sf_normalize(sf_t *a, int bits)
{
    if (a->cls != SF_NORMAL)
        return;
    int mb = sf_mant_bits(bits);
    int bias = sf_exp_bias(bits);
    int msb = sf_msb_index(a->mant, a->mant_limbs);
    if (msb < 0) {
        a->cls = SF_ZERO;
        return;
    }
    if (msb > mb) {
        int sh = msb - mb;
        sf_shr_limbs(a->mant, a->mant_limbs, sh);
        a->exp += sh;
    }
    if (a->exp > bias) {
        a->cls = SF_INF;
        return;
    }
    if (a->exp < 1 - bias) {
        int sh = (int)(1 - bias - a->exp);
        if (sh > mb + 2) {
            a->cls = SF_ZERO;
            sf_zero_limbs(a->mant, a->mant_limbs);
            return;
        }
        sf_shr_limbs(a->mant, a->mant_limbs, sh);
        a->exp += sh;
    }
}

static void sf_round(sf_t *a, int bits)
{
    if (a->cls != SF_NORMAL)
        return;
    int mb = sf_mant_bits(bits);
    int msb = sf_msb_index(a->mant, a->mant_limbs);
    if (msb <= mb)
        return;
    int guard = sf_get_bit(a->mant, a->mant_limbs, mb);
    int round = sf_get_bit(a->mant, a->mant_limbs, mb - 1);
    int sticky = 0;
    for (int i = 0; i < mb - 1; i++)
        if (sf_get_bit(a->mant, a->mant_limbs, i))
            sticky = 1;
    if (guard && (round || sticky)) {
        uint64_t one[MC_MAX_LIMBS * 2];
        sf_zero_limbs(one, a->mant_limbs);
        one[0] = 1;
        sf_add_limbs(a->mant, a->mant, one, a->mant_limbs);
        sf_normalize(a, bits);
    }
    if (sf_msb_index(a->mant, a->mant_limbs) > mb)
        sf_shr_limbs(a->mant, a->mant_limbs, sf_msb_index(a->mant, a->mant_limbs) - mb);
}

static int sf_fits_legacy_f64(const sf_t *t)
{
    if (t->cls != SF_NORMAL)
        return 1;
    if (sf_msb_index(t->mant, t->mant_limbs) > 52)
        return 0;
    if (t->exp > 1023 || t->exp < -1022)
        return 0;
    return 1;
}

static void sf_encode(uint64_t *dst, int bits, const sf_t *in)
{
    int n = sf_limb_count(bits);
    sf_zero_limbs(dst, n);

    sf_t t = *in;
    if (t.cls == SF_NORMAL) {
        sf_normalize(&t, bits);
        sf_round(&t, bits);
    }

    if (bits == 64 || t.cls == SF_ZERO || t.cls == SF_INF || t.cls == SF_NAN) {
        double d;
        sf_to_double(&t, &d);
        memcpy(dst, &d, sizeof(double));
        return;
    }

    {
        double d;
        if (sf_fits_legacy_f64(&t) && sf_to_double(&t, &d) && isfinite(d)) {
            memcpy(dst, &d, sizeof(double));
            return;
        }
    }

    int eb = sf_exp_bits(bits);
    int mb = sf_mant_bits(bits);
    int bias = sf_exp_bias(bits);

    if (t.cls == SF_NAN) {
        for (int i = 0; i < eb; i++) sf_set_bit(dst, n, bits - 2 - i, 1);
        sf_set_bit(dst, n, bits - 1, t.sign);
        dst[0] |= 1;
        return;
    }
    if (t.cls == SF_INF) {
        for (int i = 0; i < eb; i++) sf_set_bit(dst, n, bits - 2 - i, 1);
        sf_set_bit(dst, n, bits - 1, t.sign);
        return;
    }
    if (t.cls == SF_ZERO) {
        sf_set_bit(dst, n, bits - 1, t.sign);
        return;
    }

    uint64_t efield = 0;
    if (t.exp + bias > 0)
        efield = (uint64_t)(t.exp + bias);
    for (int i = 0; i < eb; i++)
        if ((efield >> (eb - 1 - i)) & 1)
            sf_set_bit(dst, n, bits - 2 - i, 1);
    sf_set_bit(dst, n, bits - 1, t.sign);
    for (int i = 0; i < mb; i++)
        sf_set_bit(dst, n, i, sf_get_bit(t.mant, t.mant_limbs, i));
}

static void sf_add(sf_t *r, const sf_t *a, const sf_t *b, int bits, int sub)
{
    memset(r, 0, sizeof(*r));
    r->mant_limbs = a->mant_limbs > b->mant_limbs ? a->mant_limbs : b->mant_limbs;
    if (r->mant_limbs < 4)
        r->mant_limbs = 4;
    if (a->cls == SF_NAN || b->cls == SF_NAN) { r->cls = SF_NAN; return; }
    if (a->cls == SF_INF || b->cls == SF_INF) {
        if (a->cls == SF_INF && b->cls == SF_INF && sub && a->sign != b->sign) {
            r->cls = SF_NAN; return;
        }
        r->cls = SF_INF;
        r->sign = a->cls == SF_INF ? a->sign : b->sign;
        if (sub && b->cls == SF_INF) r->sign ^= 1;
        return;
    }
    if (a->cls == SF_ZERO) { *r = *b; if (sub) r->sign ^= 1; return; }
    if (b->cls == SF_ZERO) { *r = *a; return; }

    sf_t x = *a, y = *b;
    if (sub) y.sign ^= 1;
    if (x.exp < y.exp) { sf_t tmp = x; x = y; y = tmp; }
    int sh = (int)(x.exp - y.exp);
    if (sh > 0)
        sf_shr_limbs(y.mant, y.mant_limbs, sh);
    if (x.sign == y.sign) {
        sf_add_limbs(r->mant, x.mant, y.mant, r->mant_limbs);
        r->sign = x.sign;
        r->exp = x.exp;
    } else if (sf_cmp_limbs(x.mant, y.mant, r->mant_limbs) >= 0) {
        sf_sub_limbs(r->mant, x.mant, y.mant, r->mant_limbs);
        r->sign = x.sign;
        r->exp = x.exp;
    } else {
        sf_sub_limbs(r->mant, y.mant, x.mant, r->mant_limbs);
        r->sign = y.sign;
        r->exp = y.exp;
    }
    r->cls = SF_NORMAL;
    sf_normalize(r, bits);
    sf_round(r, bits);
}

static void sf_mul(sf_t *r, const sf_t *a, const sf_t *b, int bits)
{
    memset(r, 0, sizeof(*r));
    r->mant_limbs = a->mant_limbs + b->mant_limbs;
    if (r->mant_limbs > MC_MAX_LIMBS * 2) r->mant_limbs = MC_MAX_LIMBS * 2;
    if (a->cls == SF_NAN || b->cls == SF_NAN) { r->cls = SF_NAN; return; }
    if ((a->cls == SF_INF && b->cls == SF_ZERO) || (a->cls == SF_ZERO && b->cls == SF_INF)) {
        r->cls = SF_NAN; return;
    }
    if (a->cls == SF_INF || b->cls == SF_INF) {
        r->cls = SF_INF; r->sign = a->sign ^ b->sign; return;
    }
    if (a->cls == SF_ZERO || b->cls == SF_ZERO) {
        r->cls = SF_ZERO; r->sign = a->sign ^ b->sign; return;
    }
    uint64_t prod[MC_MAX_LIMBS * 2];
    sf_mul_limbs(prod, a->mant, b->mant, a->mant_limbs, b->mant_limbs);
    memcpy(r->mant, prod, (size_t)r->mant_limbs * sizeof(uint64_t));
    r->sign = a->sign ^ b->sign;
    r->exp = a->exp + b->exp;
    r->cls = SF_NORMAL;
    {
        int msb = sf_msb_index(r->mant, r->mant_limbs);
        int ma = sf_msb_index(a->mant, a->mant_limbs);
        int mb_i = sf_msb_index(b->mant, b->mant_limbs);
        int expect = ma + mb_i + 1;
        if (msb > expect) {
            int sh = msb - expect;
            sf_shr_limbs(r->mant, r->mant_limbs, sh);
            r->exp += sh;
        } else if (msb >= 0 && msb < expect) {
            int sh = expect - msb;
            sf_shl_limbs(r->mant, r->mant_limbs, sh);
            r->exp -= sh;
        }
    }
    sf_normalize(r, bits);
    sf_round(r, bits);
}

static void sf_div(sf_t *r, const sf_t *a, const sf_t *b, int bits)
{
    memset(r, 0, sizeof(*r));
    r->mant_limbs = a->mant_limbs + 2;
    if (a->cls == SF_NAN || b->cls == SF_NAN) { r->cls = SF_NAN; return; }
    if (a->cls == SF_INF && b->cls == SF_INF) { r->cls = SF_NAN; return; }
    if (b->cls == SF_INF) { r->cls = SF_ZERO; r->sign = a->sign ^ b->sign; return; }
    if (a->cls == SF_INF) { r->cls = SF_INF; r->sign = a->sign ^ b->sign; return; }
    if (b->cls == SF_ZERO) { r->cls = SF_INF; r->sign = a->sign ^ b->sign; return; }
    if (a->cls == SF_ZERO) { r->cls = SF_ZERO; r->sign = a->sign ^ b->sign; return; }

    int mb = sf_mant_bits(bits);
    int wl = a->mant_limbs + 2;
    uint64_t num[MC_MAX_LIMBS * 2];
    uint64_t den[MC_MAX_LIMBS * 2];
    uint64_t q[MC_MAX_LIMBS * 2];
    memcpy(num, a->mant, (size_t)a->mant_limbs * sizeof(uint64_t));
    memcpy(den, b->mant, (size_t)b->mant_limbs * sizeof(uint64_t));
    sf_shl_limbs(num, wl, mb + 1);
    sf_zero_limbs(q, r->mant_limbs);

    for (int bit = mb; bit >= 0; bit--) {
        if (sf_cmp_limbs(num, den, wl) >= 0) {
            sf_sub_limbs(num, num, den, wl);
            q[bit / 64] |= (uint64_t)1 << (bit % 64);
        }
        int lsb = (int)(num[0] & 1);
        sf_shr_limbs(num, wl, 1);
        if (lsb)
            num[wl - 1] |= (uint64_t)1 << 63;
    }
    memcpy(r->mant, q, (size_t)r->mant_limbs * sizeof(uint64_t));
    r->sign = a->sign ^ b->sign;
    r->exp = a->exp - b->exp;
    r->cls = SF_NORMAL;
    {
        int msb = sf_msb_index(r->mant, r->mant_limbs);
        int ma = sf_msb_index(a->mant, a->mant_limbs);
        int mb_i = sf_msb_index(b->mant, b->mant_limbs);
        int expect = ma - mb_i;
        if (msb > expect) {
            int sh = msb - expect;
            sf_shr_limbs(r->mant, r->mant_limbs, sh);
            r->exp += sh;
        } else if (msb >= 0 && msb < expect) {
            int sh = expect - msb;
            sf_shl_limbs(r->mant, r->mant_limbs, sh);
            r->exp -= sh;
        }
    }
    sf_normalize(r, bits);
    sf_round(r, bits);
}

static void sf_ieee_binop(int op, int bits, uint64_t *dst, const uint64_t *a, const uint64_t *b)
{
    sf_t fa, fb, fr;
    sf_decode(a, bits, &fa);
    sf_decode(b, bits, &fb);
    switch (op) {
        case 0: sf_add(&fr, &fa, &fb, bits, 0); break;
        case 1: sf_add(&fr, &fa, &fb, bits, 1); break;
        case 2: sf_mul(&fr, &fa, &fb, bits); break;
        case 3: sf_div(&fr, &fa, &fb, bits); break;
        default: sf_add(&fr, &fa, &fb, bits, 0); break;
    }
    sf_encode(dst, bits, &fr);
}

static const char *sf_print_decimal(int bits, const uint64_t *src, char *buf, size_t bufsz)
{
    sf_t f;
    sf_decode(src, bits, &f);
    double d;
    sf_to_double(&f, &d);
    snprintf(buf, bufsz, "%.17g", d);
    return buf;
}

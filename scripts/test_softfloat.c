/* Unit tests for IEEE soft-float in numeric_rt.c */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "../src/include/numeric_rt.h"

static int tests_run;
static int tests_failed;

static void check_int(const char *name, int got, int expect)
{
    tests_run++;
    if (got != expect) {
        tests_failed++;
        fprintf(stderr, "FAIL %s: got %d expected %d\n", name, got, expect);
    } else {
        printf("PASS %s\n", name);
    }
}

static void check_near(const char *name, double got, double expect)
{
    tests_run++;
    double diff = got - expect;
    if (diff < 0) diff = -diff;
    double tol = fabs(expect) * 1e-12 + 1e-15;
    if (diff > tol) {
        tests_failed++;
        fprintf(stderr, "FAIL %s: got %.17g expected %.17g\n", name, got, expect);
    } else {
        printf("PASS %s\n", name);
    }
}

static double bits_to_double(const uint64_t *limbs)
{
    double d;
    memcpy(&d, limbs, sizeof(d));
    return d;
}

static void set_double(uint64_t *limbs, int n, double d)
{
    memset(limbs, 0, (size_t)n * sizeof(uint64_t));
    memcpy(limbs, &d, sizeof(d));
}

int main(void)
{
    uint64_t a[16], b[16], r[16];

    /* f1024 legacy-path add/mul (upper limbs zero) */
    set_double(a, 16, 1e18);
    set_double(b, 16, 1e18);
    mc_wide_float_binop_bits(0, 1024, r, a, b);
    check_near("f1024 add 1e18+1e18", bits_to_double(r), 2e18);

    set_double(a, 16, 1e-12);
    set_double(b, 16, 1e-12);
    mc_wide_float_binop_bits(0, 1024, r, a, b);
    check_near("f1024 add 1e-12+1e-12", bits_to_double(r), 2e-12);

    set_double(a, 16, 1e9);
    set_double(b, 16, 1e9);
    mc_wide_float_binop_bits(2, 1024, r, a, b);
    check_near("f1024 mul 1e9*1e9", bits_to_double(r), 1e18);

    set_double(a, 16, 10.0);
    set_double(b, 16, 4.0);
    mc_wide_float_binop_bits(3, 1024, r, a, b);
    check_near("f1024 div 10/4", bits_to_double(r), 2.5);

    set_double(a, 16, 3.0);
    set_double(b, 16, 1.0);
    mc_wide_float_binop_bits(1, 1024, r, a, b);
    check_near("f1024 sub 3-1", bits_to_double(r), 2.0);

    /* f128 add */
    set_double(a, 2, 1.5);
    set_double(b, 2, 2.25);
    mc_wide_float_binop_bits(0, 128, r, a, b);
    check_near("f128 add", bits_to_double(r), 3.75);

    /* print path */
    set_double(a, 16, 42.5);
    const char *printed = mc_wide_float_print_bits(1024, a);
    check_int("print non-empty", printed && printed[0] != '\0', 1);

    /* canonical 128-bit IEEE add (non-legacy path): 2.0 + 2.0 = 4.0 */
    memset(a, 0, sizeof(a));
    memset(b, 0, sizeof(b));
    /* binary128 LE: bit 127 sign, bits 126-112 exp=16384 (2.0), zero mantissa */
    a[1] = (uint64_t)1 << 62;
    b[1] = (uint64_t)1 << 62;
    mc_wide_float_binop_bits(0, 128, r, a, b);
    /* 4.0 in binary128 canonical: exp=16385 => bits 126 and 112 set in limb[1] */
    check_int("canonical f128 add 2+2", r[1] == ((1ULL << 62) | (1ULL << 48)), 1);

    printf("\n%d tests, %d failed\n", tests_run, tests_failed);
    return tests_failed ? 1 : 0;
}

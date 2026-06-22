#ifndef NUMERIC_RT_H
#define NUMERIC_RT_H

#include <stdint.h>

const char *mc_wide_print_bits(int bits, const uint64_t *src);
void mc_wide_pow_bits(int bits, uint64_t *dst, const uint64_t *base, uint64_t exp);
const char *mc_ftos(double v);
const char *mc_wide_float_print_bits(int bits, const uint64_t *src);
void mc_wide_float_binop_bits(int op, int bits, uint64_t *dst,
                              const uint64_t *a, const uint64_t *b);
void mc_wide_int_binop_bits(int op, int bits, uint64_t *dst,
                            const uint64_t *a, const uint64_t *b);

#endif

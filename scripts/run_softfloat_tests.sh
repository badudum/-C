#!/bin/sh
set -e
cd "$(dirname "$0")/.."

echo "=== Soft-float unit tests (IEEE 64–1024 bit) ==="
gcc -c src/runtime/numeric_rt.c -o /tmp/numeric_rt_test.o -std=c99 -O2 -lm
gcc scripts/test_softfloat.c /tmp/numeric_rt_test.o -o /tmp/test_softfloat -std=c99 -lm
/tmp/test_softfloat
echo "All soft-float unit tests OK"

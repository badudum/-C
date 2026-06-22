#!/bin/sh
# Run extended numeric type tests (64-bit int/float, promotion, sizeof).
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== Numeric tests (compile + run) ==="
rm -f mc.o mc.out
$CC "$TARGET" example/numeric_runner.minusc
./mc.out
echo "PASS: numeric_tests runtime"
echo "All numeric tests OK"

#!/bin/sh
# Run feature checklist tests (mod, compound assign, break/continue, switch).
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== Feature tests (compile + run) ==="
rm -f mc.o mc.out
$CC "$TARGET" example/feature_runner.minusc
./mc.out
echo "PASS: feature_tests runtime"
echo "All feature tests OK"

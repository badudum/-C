#!/bin/sh
# Run borrow checker tests: positive runtime suite + negative compile-fail cases.
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== Borrow positive (compile + run) ==="
rm -f mc.o mc.out
$CC "$TARGET" example/borrow_runner.minusc
./mc.out
echo "PASS: borrow_tests runtime"

echo ""
echo "=== Borrow negative (must fail compile) ==="
fail=0
pass=0
for f in example/borrow_fail/*.minusc; do
    name=$(basename "$f")
    if $CC "$TARGET" "$f" >/dev/null 2>&1; then
        echo "FAIL: $name compiled but should have been rejected"
        fail=$((fail + 1))
    else
        echo "PASS: $name rejected at compile time"
        pass=$((pass + 1))
    fi
done

echo ""
echo "Negative: $pass passed, $fail failed"
if [ "$fail" -ne 0 ]; then
    exit 1
fi
echo "All borrow tests OK"

#!/bin/sh
# Run OOP / method tests: positive runtime suite + negative compile-fail cases.
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== OOP positive (compile + run) ==="
rm -f mc.o mc.out
$CC "$TARGET" example/oop_runner.minusc
./mc.out | tee /tmp/oop_test_out.txt
if grep -q '^FAIL:' /tmp/oop_test_out.txt; then
    echo "FAIL: oop_tests reported failures"
    grep '^FAIL:' /tmp/oop_test_out.txt
    exit 1
fi
echo "PASS: oop_tests runtime"

echo ""
echo "=== OOP negative (must fail compile) ==="
fail=0
pass=0
for f in example/oop_fail/*.minusc; do
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
echo "All OOP tests OK"

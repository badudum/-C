#!/bin/sh
set -e
TARGET="${1:---arm64}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CC="./minusC.out $TARGET"
fail() { echo "FAIL: $1"; exit 1; }

echo "=== Generic positive (compile + run) ==="
$CC example/generic_runner.minusc
./mc.out | tee /tmp/generic_out.txt

grep -q "PASS: Box<int> stores int" /tmp/generic_out.txt || fail "generic-1"
grep -q "PASS: Box<Point> nested cust field" /tmp/generic_out.txt || fail "generic-2"
grep -q "PASS: Box_int and Box_Point distinct instances" /tmp/generic_out.txt || fail "generic-3"
echo "PASS: generic_tests runtime"

echo ""
echo "=== Generic negative (must fail compile) ==="
neg_pass=0
neg_fail=0
for f in example/generic_fail/*.minusc; do
    base=$(basename "$f")
    if $CC "$f" 2>/dev/null; then
        echo "FAIL: $base should have been rejected"
        neg_fail=$((neg_fail + 1))
    else
        echo "PASS: $base rejected at compile time"
        neg_pass=$((neg_pass + 1))
    fi
done
echo "Negative: $neg_pass passed, $neg_fail failed"
[ "$neg_fail" -eq 0 ] || exit 1
echo "All generic tests OK"

#!/bin/sh
set -e
TARGET="${1:---arm64}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CC="./minusC.out $TARGET"
fail() { echo "FAIL: $1"; exit 1; }

echo "=== Interface positive (compile + run) ==="
$CC example/interface_runner.minusc
./mc.out | tee /tmp/interface_out.txt

grep -q "PASS: Circle implements Drawable" /tmp/interface_out.txt || fail "iface-1"
grep -q "PASS: Circle.draw virtual" /tmp/interface_out.txt || fail "iface-2"
echo "PASS: interface_tests runtime"

echo ""
echo "=== Interface negative (must fail compile) ==="
neg_pass=0
neg_fail=0
for f in example/interface_fail/*.minusc; do
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
echo "All interface tests OK"

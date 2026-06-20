#!/bin/sh
set -e
TARGET="${1:---arm64}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CC="./minusC.out $TARGET"
fail() { echo "FAIL: $1"; exit 1; }

echo "=== Polymorphism positive (compile + run) ==="
$CC example/poly_runner.minusc
./mc.out | tee /tmp/poly_out.txt

echo ""
echo "--- core poly tests ---"
grep -q "PASS: Dog.speak override" /tmp/poly_out.txt || fail "poly-1"
grep -q "PASS: virtual dispatch through helper" /tmp/poly_out.txt || fail "poly-2"
grep -q "PASS: super.init + derived init" /tmp/poly_out.txt || fail "poly-3"
grep -q "PASS: static speak on Animal vs Dog" /tmp/poly_out.txt || fail "poly-4"
grep -q "PASS: sizeof includes vtable header" /tmp/poly_out.txt || fail "poly-5"

echo "--- complex / edge poly tests ---"
grep -q "PASS: 3-level super chain + dual virtual overrides" /tmp/poly_out.txt || fail "poly-6"
grep -q "PASS: super.id() inside virtual PolyMid.id" /tmp/poly_out.txt || fail "poly-7"
grep -q "PASS: override slotA, inherit slotB" /tmp/poly_out.txt || fail "poly-8"
grep -q "PASS: same helper, different vtables" /tmp/poly_out.txt || fail "poly-9"
grep -q "PASS: two Dog instances same vtable layout" /tmp/poly_out.txt || fail "poly-10"
grep -q "PASS: derived reads base field via virtual method" /tmp/poly_out.txt || fail "poly-11"
grep -q "PASS: sizeof grows with derived fields + vtable" /tmp/poly_out.txt || fail "poly-12"
grep -q "PASS: static tag + virtual speak coexist" /tmp/poly_out.txt || fail "poly-13"
grep -q "PASS: repeated virtual dispatch consistent" /tmp/poly_out.txt || fail "poly-14"
grep -q "PASS: PolyMid and PolyLeaf direct virtual id" /tmp/poly_out.txt || fail "poly-15"

echo "PASS: poly_tests runtime"

echo ""
echo "=== Polymorphism negative (must fail compile) ==="
neg_pass=0
neg_fail=0
for f in example/poly_fail/*.minusc; do
    base=$(basename "$f")
    if $CC "$f" 2>/dev/null; then
        echo "FAIL: $base should have been rejected"
        neg_fail=$((neg_fail + 1))
    else
        echo "PASS: $base rejected at compile time"
        neg_pass=$((neg_pass + 1))
    fi
done
echo ""
echo "Negative: $neg_pass passed, $neg_fail failed"
[ "$neg_fail" -eq 0 ] || exit 1
echo "All polymorphism tests OK"

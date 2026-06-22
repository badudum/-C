#!/bin/sh
set -e
cd "$(dirname "$0")/.."
TARGET="${1:---arm64}"
COMPILER="./minusC.out ${TARGET}"

echo "=== Advanced feature tests (compile + run) ==="
$COMPILER example/advanced_tests.minusc
OUT=$(./mc.out 2>&1 || true)
echo "$OUT"
echo "$OUT" | grep -q "PASS: constexpr binop fold" || exit 1
echo "$OUT" | grep -q "PASS: try else branch" || exit 1
echo "$OUT" | grep -q "PASS: try ok path" || exit 1
echo "$OUT" | grep -q "PASS: arena rent" || exit 1
echo "$OUT" | grep -q "PASS: pool rent" || exit 1
echo "$OUT" | grep -q "PASS: std.math" || exit 1
echo "All advanced feature tests OK"

#!/bin/sh
set -e
cd "$(dirname "$0")/.."
TARGET="${1:---arm64}"
COMPILER="./minusC.out ${TARGET}"

echo "=== Remaining feature tests (compile + run) ==="
$COMPILER example/remaining_tests.minusc
OUT=$(./mc.out 2>&1 || true)
echo "$OUT"
echo "$OUT" | grep -q "PASS: Box<int> extends Animal" || exit 1
echo "$OUT" | grep -q "PASS: Vec2 operator +" || exit 1
echo "$OUT" | grep -q "PASS: Option sum type" || exit 1
echo "$OUT" | grep -q "PASS: nested function call" || exit 1
echo "$OUT" | grep -q "PASS: closure capture by value" || exit 1
echo "$OUT" | grep -q "PASS: closure call-site rebind" || exit 1

echo "=== Diagnostics check mode ==="
$COMPILER --check example/remaining_tests.minusc 2>&1 | grep -q "Check OK" || exit 1

echo "=== Diagnostics type mismatch (expect failure) ==="
cat > /tmp/minusc_diag_fail.minusc <<'EOF'
bad = ({x} int) function {
    {s} str = "hi";
    {n} int = s + 1;
    return 0;
} int;
EOF
if $COMPILER --check /tmp/minusc_diag_fail.minusc 2>/tmp/minusc_diag_err.txt; then
    echo "FAIL: expected compile error for str+int"
    exit 1
fi
grep -q "invalid operands" /tmp/minusc_diag_err.txt || exit 1
grep -q "hint:" /tmp/minusc_diag_err.txt || exit 1

echo "All remaining feature tests OK"

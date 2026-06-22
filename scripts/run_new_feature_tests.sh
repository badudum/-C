#!/bin/sh
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== New feature tests ==="
rm -f mc.o mc.out io_rt.o numeric_rt.o
$CC "$TARGET" example/new_feature_runner.minusc
./mc.out
echo "PASS: new_feature_tests"

echo ""
echo "=== New feature negative (immportal reassign) ==="
if $CC "$TARGET" example/new_feature_fail/immportal_reassign.minusc >/dev/null 2>&1; then
    echo "FAIL: immportal_reassign compiled"
    exit 1
fi
echo "PASS: immportal_reassign rejected"

echo ""
echo "=== New feature negative (immportal field reassign) ==="
if $CC "$TARGET" example/new_feature_fail/immportal_field_reassign.minusc >/dev/null 2>&1; then
    echo "FAIL: immportal_field_reassign compiled"
    exit 1
fi
echo "PASS: immportal_field_reassign rejected"

echo ""
echo "=== New feature negative (const array index OOB) ==="
if $CC "$TARGET" example/new_feature_fail/array_const_oob.minusc >/dev/null 2>&1; then
    echo "FAIL: array_const_oob compiled"
    exit 1
fi
echo "PASS: array_const_oob rejected"
echo "All new feature tests OK"

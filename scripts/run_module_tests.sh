#!/bin/sh
set -e
TARGET="${1:---arm64}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

CC="./minusC.out $TARGET"
fail() { echo "FAIL: $1"; exit 1; }

echo "=== Module positive (compile + run) ==="
$CC example/module_runner.minusc
./mc.out | tee /tmp/module_out.txt

grep -q "PASS: module_helper.moduleAnswer" /tmp/module_out.txt || fail "mod-1 answer"
grep -q "PASS: module_helper.moduleAdd" /tmp/module_out.txt || fail "mod-1 add"
echo "PASS: module_tests runtime"

echo ""
echo "=== Module negative (unqualified call after qualified reference) ==="
if $CC example/module_fail/unqualified_call.minusc 2>/dev/null; then
    echo "FAIL: unqualified_call should have been rejected"
    exit 1
fi
echo "PASS: unqualified_call rejected"
echo "All module tests OK"

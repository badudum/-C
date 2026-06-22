#!/bin/sh
# Run all minusC test suites.
set -e
cd "$(dirname "$0")/.."
TARGET="${1:---arm64}"

for t in test-borrow test-cust test-oop test-heap-oop test-poly test-generic test-interface test-numeric test-feature test-io test-new-feature test-module test-advanced test-remaining; do
    echo ""
    make "$t" TARGET="$TARGET"
done
echo ""
echo "All test suites passed"

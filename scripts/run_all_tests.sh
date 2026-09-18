#!/bin/sh
# Run all minusC test suites.
set -e
cd "$(dirname "$0")/.."
TARGET="${1:---arm64}"

for t in \
    scripts/run_borrow_tests.sh \
    scripts/run_cust_tests.sh \
    scripts/run_oop_tests.sh \
    scripts/run_heap_oop_tests.sh \
    scripts/run_poly_tests.sh \
    scripts/run_generic_tests.sh \
    scripts/run_interface_tests.sh \
    scripts/run_numeric_tests.sh \
    scripts/run_feature_tests.sh \
    scripts/run_io_tests.sh \
    scripts/run_new_feature_tests.sh \
    scripts/run_module_tests.sh \
    scripts/run_advanced_tests.sh \
    scripts/run_remaining_tests.sh \
    scripts/run_softfloat_tests.sh
do
    echo ""
    echo "######## $t ########"
    sh "$t" "$TARGET"
done
echo ""
echo "All test suites passed"

#!/bin/sh
# I/O runtime tests (file + stdin). Uses relative paths under example/.
set -e
cd "$(dirname "$0")/.."
CC="./minusC.out"
TARGET="${1:---arm64}"

if [ ! -x "$CC" ]; then
    echo "Building compiler..."
    make
fi

echo "=== I/O tests (file + security) ==="
rm -f mc.o mc.out io_rt.o numeric_rt.o example/_io_tmp.txt
$CC "$TARGET" example/io_runner.minusc
./mc.out
echo "PASS: io_runner"

echo ""
echo "=== I/O stdin test (piped) ==="
rm -f mc.o mc.out io_rt.o numeric_rt.o
$CC "$TARGET" example/io_stdin_runner.minusc
out=$(printf 'alice\n' | ./mc.out)
echo "$out" | grep -q 'ReadLine=alice'
echo "$out" | grep -q 'PASS: ReadLine'
echo "PASS: io_stdin_runner"

rm -f example/_io_tmp.txt
echo "All I/O tests OK"

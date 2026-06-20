exec = minusC.out
sources = $(wildcard src/*.c)
sources += $(wildcard src/*/*.c)
flags = -g -std=gnu99
archs = arm64 x86_64

# Per-architecture object files and binaries for universal build
objects_arm64 = $(sources:.c=.arm64.o)
objects_x86_64 = $(sources:.c=.x86_64.o)

.PHONY: all clean install test-borrow test-cust test-oop test-heap-oop test-poly test-generic test-interface

all: $(exec)

$(exec): minusC.arm64 minusC.x86_64
	lipo -create -output $(exec) minusC.arm64 minusC.x86_64

minusC.arm64: $(objects_arm64)
	gcc $(objects_arm64) $(flags) -arch arm64 -o $@

minusC.x86_64: $(objects_x86_64)
	gcc $(objects_x86_64) $(flags) -arch x86_64 -o $@

%.arm64.o: %.c
	gcc -c $(flags) -arch arm64 $< -o $@

%.x86_64.o: %.c
	gcc -c $(flags) -arch x86_64 $< -o $@

install:
	$(MAKE)
	cp ./$(exec) /usr/local/bin/minusC

test-borrow: $(exec)
	sh scripts/run_borrow_tests.sh --arm64

test-cust: $(exec)
	sh scripts/run_cust_tests.sh --arm64

test-oop: $(exec)
	sh scripts/run_oop_tests.sh --arm64

test-heap-oop: $(exec)
	sh scripts/run_heap_oop_tests.sh --arm64

test-poly: $(exec)
	sh scripts/run_poly_tests.sh --arm64

test-generic: $(exec)
	sh scripts/run_generic_tests.sh --arm64

test-interface: $(exec)
	sh scripts/run_interface_tests.sh --arm64

clean:
	-rm -f $(objects_arm64) $(objects_x86_64)
	-rm -f minusC.arm64 minusC.x86_64 $(exec)
	-rm -f *.o
	-rm -f src/*.o src/*/*.o

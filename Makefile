exec = minusC
sources = $(wildcard src/*.c)
sources += $(wildcard src/*/*.c)
# gui_rt.c is only linked into compiled minusC programs, not the compiler.
sources := $(filter-out src/runtime/gui_rt.c,$(sources))
objects = $(sources:.c=.o)
flags = -g -std=gnu99

UNAME_M := $(shell uname -m)
ifeq ($(UNAME_M),x86_64)
TEST_TARGET := --x86_64
else ifeq ($(UNAME_M),amd64)
TEST_TARGET := --x86_64
else
TEST_TARGET := --arm64
endif

.PHONY: all clean install hooks test-borrow test-cust test-oop test-heap-oop test-poly test-generic test-interface test-numeric test-softfloat test-feature test-io test-new-feature test-module test-advanced test-remaining test-all

all: $(exec)

$(exec): $(objects)
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c
	gcc -c $(flags) $< -o $@

hooks:
	sh scripts/install-githooks.sh

install:
	$(MAKE)
	$(MAKE) hooks
	cp ./$(exec) /usr/local/bin/minusC

test-borrow: $(exec)
	sh scripts/run_borrow_tests.sh $(TEST_TARGET)

test-cust: $(exec)
	sh scripts/run_cust_tests.sh $(TEST_TARGET)

test-oop: $(exec)
	sh scripts/run_oop_tests.sh $(TEST_TARGET)

test-heap-oop: $(exec)
	sh scripts/run_heap_oop_tests.sh $(TEST_TARGET)

test-poly: $(exec)
	sh scripts/run_poly_tests.sh $(TEST_TARGET)

test-generic: $(exec)
	sh scripts/run_generic_tests.sh $(TEST_TARGET)

test-interface: $(exec)
	sh scripts/run_interface_tests.sh $(TEST_TARGET)

test-numeric: $(exec)
	sh scripts/run_numeric_tests.sh $(TEST_TARGET)

test-softfloat:
	sh scripts/run_softfloat_tests.sh

test-feature: $(exec)
	sh scripts/run_feature_tests.sh $(TEST_TARGET)

test-io: $(exec)
	sh scripts/run_io_tests.sh $(TEST_TARGET)

test-new-feature: $(exec)
	sh scripts/run_new_feature_tests.sh $(TEST_TARGET)

test-module: $(exec)
	sh scripts/run_module_tests.sh $(TEST_TARGET)

test-advanced: $(exec)
	sh scripts/run_advanced_tests.sh $(TEST_TARGET)

test-remaining: $(exec)
	sh scripts/run_remaining_tests.sh $(TEST_TARGET)

test-all: $(exec)
	sh scripts/run_softfloat_tests.sh
	sh scripts/run_all_tests.sh $(TEST_TARGET)

clean:
	-rm -f $(objects)
	-rm -f $(exec) minusC.out minusC.arm64 minusC.x86_64
	-rm -f *.o
	-rm -f src/*.o src/*/*.o

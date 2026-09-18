exec = minusC
sources = $(wildcard src/*.c)
sources += $(wildcard src/*/*.c)
# gui_rt.c is only linked into compiled minusC programs, not the compiler.
sources := $(filter-out src/runtime/gui_rt.c,$(sources))
objects = $(sources:.c=.o)

UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Host CPU: x86_64 or arm64. Override with: make HOST_ARCH=x86_64
ifndef HOST_ARCH
ifeq ($(UNAME_M),x86_64)
HOST_ARCH := x86_64
else ifeq ($(UNAME_M),amd64)
HOST_ARCH := x86_64
else ifeq ($(UNAME_M),i386)
HOST_ARCH := x86_64
else ifeq ($(UNAME_M),i686)
HOST_ARCH := x86_64
else ifeq ($(UNAME_M),arm64)
HOST_ARCH := arm64
else ifeq ($(UNAME_M),aarch64)
HOST_ARCH := arm64
else
HOST_ARCH := $(UNAME_M)
endif
endif

# Apple gcc/clang needs -arch; Linux gcc already targets the host CPU.
ifeq ($(UNAME_S),Darwin)
ARCH_FLAGS := -arch $(HOST_ARCH)
else
ARCH_FLAGS :=
endif

flags = -g -std=gnu99 $(ARCH_FLAGS)

.PHONY: all clean install hooks

all: $(exec)

$(exec): $(objects)
	@echo "Linking $(exec) for $(HOST_ARCH) ($(UNAME_S) $(UNAME_M))"
	gcc $(objects) $(flags) -o $(exec)

%.o: %.c
	gcc -c $(flags) $< -o $@

hooks:
	sh scripts/install-githooks.sh

install:
	$(MAKE)
	$(MAKE) hooks
	cp ./$(exec) /usr/local/bin/minusC

clean:
	-rm -f $(objects)
	-rm -f $(exec) minusC.out minusC.arm64 minusC.x86_64
	-rm -f *.o
	-rm -f src/*.o src/*/*.o

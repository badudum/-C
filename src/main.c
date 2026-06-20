#include <stdio.h>
#include <string.h>
#include "include/lexer.h"
#include "include/minusC.h"
#include "include/assembly_target.h"

int main(int argc, char* argv[])
{
    int argi = 1;
    assembly_target_t target = assembly_target_detect_host();

    while (argi < argc && argv[argi][0] == '-') {
        if (strcmp(argv[argi], "--target") == 0 || strcmp(argv[argi], "-m") == 0) {
            if (argi + 1 >= argc) {
                fprintf(stderr, "Missing value for %s\n", argv[argi]);
                return 1;
            }
            target = assembly_target_parse(argv[argi + 1]);
            argi += 2;
            continue;
        }
        if (strcmp(argv[argi], "--x86_64") == 0 || strcmp(argv[argi], "-x86") == 0) {
            target = ASSEMBLY_TARGET_X86_64;
            argi++;
            continue;
        }
        if (strcmp(argv[argi], "--arm64") == 0 || strcmp(argv[argi], "-arm64") == 0) {
            target = ASSEMBLY_TARGET_AARCH64;
            argi++;
            continue;
        }
        fprintf(stderr, "Unknown option: %s\n", argv[argi]);
        return 1;
    }

    if (argi >= argc) {
        printf("Usage: %s [--target arm64|x86_64] <input.minusc>\n", argv[0]);
        printf("Default target: host CPU (%s)\n", assembly_target_name(assembly_target_detect_host()));
        return 1;
    }

    assembly_target_set(target);
    minusCompile_file(argv[argi]);
    return 0;
}

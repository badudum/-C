#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include "include/lexer.h"
#include "include/minusC.h"
#include "include/assembly_target.h"
#include "include/preprocess.h"
#include "include/errors.h"

static int is_dir_path(const char *path)
{
    struct stat st;
    return path && stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static void init_std_library_root(const char *argv0)
{
    const char *env = getenv("MINUSC_STD");
    if (env && is_dir_path(env)) {
        preprocess_set_std_root(env);
        return;
    }
    if (argv0) {
        char *copy = strdup(argv0);
        if (copy) {
            char *dir = dirname(copy);
            size_t dl = strlen(dir);
            char *candidate = calloc(dl + 8, 1);
            if (candidate) {
                memcpy(candidate, dir, dl);
                if (dl == 0 || dir[dl - 1] != '/')
                    strcat(candidate, "/");
                strcat(candidate, "../std");
                if (is_dir_path(candidate)) {
                    preprocess_set_std_root(candidate);
                    free(candidate);
                    free(copy);
                    return;
                }
                free(candidate);
            }
            free(copy);
        }
    }
    if (is_dir_path("std"))
        preprocess_set_std_root("std");
}

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
        if (strcmp(argv[argi], "--check") == 0) {
            if (argi + 1 >= argc) {
                fprintf(stderr, "Missing file for --check\n");
                return 1;
            }
            init_std_library_root(argv[0]);
            assembly_target_set(target);
            if (strcmp(argv[argi + 1], "--json-diagnostics") == 0) {
                errors_set_json_diagnostics(1);
                if (argi + 2 >= argc) {
                    fprintf(stderr, "Missing file for --check\n");
                    return 1;
                }
                minusCheck_file(argv[argi + 2]);
                return 0;
            }
            minusCheck_file(argv[argi + 1]);
            return 0;
        }
        if (strcmp(argv[argi], "--json-diagnostics") == 0) {
            errors_set_json_diagnostics(1);
            argi++;
            continue;
        }
        if (strcmp(argv[argi], "--list-targets") == 0) {
            printf("Supported codegen targets:\n");
            printf("  arm64   Apple Silicon / AArch64 (default on Apple arm64 hosts)\n");
            printf("  x86_64  Intel / AMD 64-bit\n");
            printf("Host OS is detected at compiler build time (macOS or Linux).\n");
            printf("Linux x86_64: ELF output with _start entry; link via gcc -e _start.\n");
            printf("macOS: Mach-O output with universal minusC.out (arm64 + x86_64).\n");
            return 0;
        }
        fprintf(stderr, "Unknown option: %s\n", argv[argi]);
        return 1;
    }

    if (argi >= argc) {
        printf("Usage: %s [--target arm64|x86_64] [--check file] [--json-diagnostics] <input.minusc>\n", argv[0]);
        printf("Default target: host CPU (%s)\n", assembly_target_name(assembly_target_detect_host()));
        return 1;
    }

    init_std_library_root(argv[0]);
    assembly_target_set(target);
    minusCompile_file(argv[argi]);
    return 0;
}

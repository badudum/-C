#define _DEFAULT_SOURCE
#include "include/minusC.h"
#include "include/preprocess.h"
#include "include/list.h"
#include "include/cust.h"
#include "include/assembly_target.h"
#include "include/assembly_emit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void link_object(assembly_target_t target)
{
    if (assembly_os_get() == ASSEMBLY_OS_LINUX) {
        if (target == ASSEMBLY_TARGET_X86_64) {
            command("gcc -c mc.s -o mc.o");
            command("gcc -no-pie mc.o -o mc.out -lpthread -lc");
        } else {
            command("gcc -c mc.s -o mc.o");
            command("gcc -nostdlib -no-pie mc.o -o mc.out -lpthread -lc");
        }
    } else if (target == ASSEMBLY_TARGET_X86_64) {
        command("as -arch x86_64 mc.s -o mc.o");
        command("ld -macos_version_min 15.0.0 mc.o -o mc.out -lSystem -lpthread "
                  "-syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch x86_64");
    } else {
        command("as mc.s -o mc.o");
        command("ld -macos_version_min 15.0.0 mc.o -o mc.out -lSystem -lpthread "
                  "-syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64");
    }
}

void compile(char * src)
{
    assembly_target_t target = assembly_target_get();
    cust_registry_reset();

    lexer_t* lexer = init_lexer(src);

    parser_t* parser = init_parser(lexer);
    AST_t * root = parse(parser);

    visitor_t*  visitor = init_visitor();
    AST_t * optimized_root = visitor_visit(visitor, root, init_list(sizeof(struct AST_S*)), init_stackframe());

    char * ass = assemble_root(optimized_root, init_list(sizeof(struct AST_S*)));

    assembly_patch_linux_output(ass);

    write_file("mc.s", ass);
    write_file("mc.s.txt", ass);

    link_object(target);
}

void minusCompile_file(const char* filename)
{
    char* src = read_file(filename);

    /* Path to current file (for resolving relative references). Use as-is; avoid realpath to prevent blocking. */
    char* current_path = strdup(filename);
    if (!current_path) {
        free(src);
        return;
    }

    dynamic_list_t* included = init_list(sizeof(char*));
    char* expanded = preprocess_source(src, current_path, included);
    free(src);
    free(current_path);
    list_free(included);

    if (!expanded) {
        fprintf(stderr, "Preprocessing failed\n");
        return;
    }

    compile(expanded);
    free(expanded);
}




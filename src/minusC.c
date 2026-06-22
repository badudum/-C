#define _DEFAULT_SOURCE
#include "include/minusC.h"
#include "include/preprocess.h"
#include "include/list.h"
#include "include/cust.h"
#include "include/generic.h"
#include "include/interface.h"
#include "include/module.h"
#include "include/assembly_target.h"
#include "include/assembly_emit.h"
#include "include/errors.h"
#include "include/AST.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_compile_check_only = 0;

void compile_set_check_only(int enabled)
{
    g_compile_check_only = enabled ? 1 : 0;
}

static void link_object(assembly_target_t target)
{
    command("gcc -c src/runtime/numeric_rt.c -o numeric_rt.o -std=c99 -O2");
    command("gcc -c src/runtime/io_rt.c -o io_rt.o -std=c99 -O2");
    command("gcc -c src/runtime/arena_rt.c -o arena_rt.o -std=c99 -O2");
    if (assembly_os_get() == ASSEMBLY_OS_LINUX) {
        if (target == ASSEMBLY_TARGET_X86_64) {
            command("gcc -c mc.s -o mc.o");
            command("gcc -no-pie mc.o numeric_rt.o io_rt.o arena_rt.o -o mc.out -lpthread -lc -lm -e _start");
        } else {
            command("gcc -c mc.s -o mc.o");
            command("gcc -nostdlib -no-pie mc.o numeric_rt.o io_rt.o arena_rt.o -o mc.out -lpthread -lc -lm -e _start");
        }
    } else if (target == ASSEMBLY_TARGET_X86_64) {
        command("as -arch x86_64 mc.s -o mc.o");
        command("ld -macos_version_min 15.0.0 mc.o numeric_rt.o io_rt.o arena_rt.o -o mc.out -lSystem -lpthread -lm "
                  "-syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch x86_64");
    } else {
        command("as -arch arm64 mc.s -o mc.o");
        command("ld -macos_version_min 15.0.0 mc.o numeric_rt.o io_rt.o arena_rt.o -o mc.out -lSystem -lpthread -lm "
                  "-syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64");
    }
}

static int func_exists_in_root(AST_t *root, const char *fname)
{
    if (!root || !root->children || !fname)
        return 0;
    for (unsigned int j = 0; j < root->children->size; j++) {
        AST_t *cand = (AST_t *)root->children->items[j];
        if (cand && cand->type == FUNC_AST && cand->name && strcmp(cand->name, fname) == 0)
            return 1;
    }
    return 0;
}

static void hoist_nested_func_literals(AST_t *root, visitor_t *visitor)
{
    if (!root || !visitor || !visitor->object || !visitor->object->children)
        return;
    for (unsigned int i = 0; i < visitor->object->children->size; i++) {
        AST_t *fn = (AST_t *)visitor->object->children->items[i];
        if (!fn || fn->type != FUNC_AST || fn->id != AST_NESTED_FUNC_LITERAL || !fn->name)
            continue;
        if (!func_exists_in_root(root, fn->name))
            list_enqueue(root->children, fn);
    }
}

void compile(char * src, const char *filename)
{
    assembly_target_t target = assembly_target_get();
    cust_registry_reset();
    generic_registry_reset();
    interface_registry_reset();

    lexer_t* lexer = init_lexer(src, filename ? filename : "<stdin>");

    parser_t* parser = init_parser(lexer);
    AST_t * root = parse(parser);
    generic_append_instantiated_to_root(root);

    visitor_t*  visitor = init_visitor();
    AST_t * optimized_root = visitor_visit(visitor, root, init_list(sizeof(struct AST_S*)), init_stackframe());

    hoist_nested_func_literals(optimized_root, visitor);

    if (g_compile_check_only) {
        fprintf(stderr, "Check OK: %s\n", filename ? filename : "<stdin>");
        return;
    }

    char * ass = assemble_root(optimized_root, init_list(sizeof(struct AST_S*)));

    assembly_patch_linux_output(ass);
    assembly_patch_macos_x86_output(ass);
    assembly_patch_macos_runtime_symbols(ass);

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

    module_registry_reset();

    dynamic_list_t* included = init_list(sizeof(char*));
    char* expanded = preprocess_source(src, current_path, included);
    free(src);
    free(current_path);
    list_free(included);

    if (!expanded) {
        fprintf(stderr, "Preprocessing failed\n");
        return;
    }

    compile(expanded, filename);
    free(expanded);
}

void minusCheck_file(const char* filename)
{
    compile_set_check_only(1);
    minusCompile_file(filename);
    compile_set_check_only(0);
}




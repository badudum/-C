#include "include/minusC.h"
#include <stdio.h>

void compile(char * src)
{

    lexer_t* lexer = init_lexer(src);

    parser_t* parser = init_parser(lexer);
    AST_t * root = parse(parser);

    
    visitor_t*  visitor = init_visitor();
    AST_t * optimized_root = visitor_visit(visitor, root, init_list(sizeof(struct AST_S*)), init_stackframe());

    char * ass = assemble_root(optimized_root, init_list(sizeof(struct AST_S*)));

    write_file("mc.s", ass);
    write_file("mc.s.txt", ass);

    // command("as mc.s -o mc.o"); // for mac
    command("arm-linux-gnueabi-gcc-12 mc.s -o mc.o"); // for x86 linux
    command("ld -macos_version_min 14.0.0 mc.o -o mc.out -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e _start -arch arm64 ");

    
}

void minusCompile_file(const char* filename)
{
    char * src = read_file(filename);

    compile(src);
    free(src);
}




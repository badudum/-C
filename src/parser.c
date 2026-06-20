#include "include/parser.h"
#include <stdio.h>
#include "include/AST.h"
#include "include/token.h"
#include "include/cust.h"
#include "include/types.h"
#include "include/generic.h"
#include "include/interface.h"
#include "include/errors.h"
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

AST_t *parse_compound(parser_t *parser);

static int parser_read_datatype(parser_t *parser);


AST_t * parse_int(parser_t * parser)
{
    int value = atoi(parser->token->value);
    AST_t * ast = parser_make_ast(parser, INT_AST);
    ast->int_value = value;
    parser_next(parser, INT_TOKEN);

    return ast;
}



parser_t * init_parser(lexer_t * lexer)
{
    parser_t * parser = calloc(1, sizeof(struct PARSER_S));
    parser->lexer = lexer;
    parser->token = lexer_get_next_token(lexer);
 
    return parser;
}

token_t * parser_next(parser_t * parser, int token_type)
{
    // printf("[Next]Current token : %s\n", parser->token->value);
    if(parser->token->type == token_type)
    {
        parser->token = lexer_get_next_token(parser->lexer);
    }
    else
    {
        compile_error_parser(parser,
            "Unexpected token '%s' (type %d), expected token type %d",
            parser->token->value,
            parser->token->type,
            token_type);
    }
}

AST_t * parse(parser_t* parser)
{
    return parse_compound(parser);
}


// =================================================================================================
// THIS IS WHERE THE IMPORTANT STUFF HAPPENS (SYNTACTIC STUFF).
// MODIFY BELOW HERE AND THE LEXER TO ACHIEVE DIFFERENT STYLES OF CODE IG.
// =================================================================================================

AST_t * parse_dupe(parser_t * parser);

static int parser_is_cust_or_class_token(parser_t *parser)
{
    return parser->token->type == CUST_TOKEN || parser->token->type == CLASS_TOKEN;
}

static int parser_brace_starts_var_decl(parser_t *parser)
{
    if (parser->token->type != LBRACE_TOKEN)
        return 0;
    lexer_t *lex = parser->lexer;
    if (!isalpha((unsigned char)lex->c))
        return 0;
    int i = 1;
    while (isalnum((unsigned char)lexer_peek(lex, i)))
        i++;
    return lexer_peek(lex, i) == '}';
}

static int parser_token_starts_datatype(parser_t *parser)
{
    return parser->token->type == ID_TOKEN || parser->token->type == BITAND_TOKEN;
}

static int parser_type_param_index(parser_t *parser, const char *name)
{
    if (!parser->generic_params || !name)
        return -1;
    for (unsigned int i = 0; i < parser->generic_params->size; i++) {
        char *p = (char *)parser->generic_params->items[i];
        if (p && strcmp(p, name) == 0)
            return (int)i;
    }
    return -1;
}

static dynamic_list_t *parser_parse_type_args(parser_t *parser)
{
    parser_next(parser, LT_TOKEN);
    dynamic_list_t *args = init_list(sizeof(void *));
    list_enqueue(args, (void *)(intptr_t)parser_read_datatype(parser));
    while (parser->token->type == COMMA_TOKEN) {
        parser_next(parser, COMMA_TOKEN);
        list_enqueue(args, (void *)(intptr_t)parser_read_datatype(parser));
    }
    parser_next(parser, GT_TOKEN);
    return args;
}

static int parser_read_generic_instantiated(parser_t *parser, const char *base_name)
{
    dynamic_list_t *args = parser_parse_type_args(parser);
    AST_t loc;
    memset(&loc, 0, sizeof(loc));
    ast_set_loc_from_parser(&loc, parser);
    int cid = generic_instantiate(base_name, args, &loc);
    list_free_shallow(args);
    return cid;
}

static int parser_read_datatype(parser_t *parser)
{
    if (parser->token->type == BITAND_TOKEN) {
        parser_next(parser, BITAND_TOKEN);
        if (parser->token->type == ID_TOKEN && strcmp(parser->token->value, "mut") == 0) {
            parser_next(parser, ID_TOKEN);
            if (parser->token->type != ID_TOKEN || strcmp(parser->token->value, "adr") != 0) {
                compile_error_parser(parser, "expected 'adr' after '&mut'");
            }
            parser_next(parser, ID_TOKEN);
            return TYPE_ADR_MUTREF;
        }
        if (parser->token->type != ID_TOKEN || strcmp(parser->token->value, "adr") != 0) {
            compile_error_parser(parser, "expected 'adr' after '&'");
        }
        parser_next(parser, ID_TOKEN);
        return TYPE_ADR_SHREF;
    }
    if (parser->token->type != ID_TOKEN) {
        compile_error_parser(parser, "expected type name");
    }
    char *type_name = mkstr(parser->token->value);
    parser_next(parser, ID_TOKEN);

    if (parser->token->type == LT_TOKEN) {
        if (generic_lookup_template(type_name) >= 0) {
            int cid = parser_read_generic_instantiated(parser, type_name);
            free(type_name);
            return MAKE_CUST_TYPE(cid);
        }
        if (strcmp(type_name, "Array") == 0) {
            int elem = parser_read_datatype(parser);
            if (elem == TYPE_UNKNOWN || elem == TYPE_ARRAY || IS_TYPE_PARAM(elem)) {
                compile_error_parser(parser, "invalid Array element type");
            }
            free(type_name);
            return TYPE_ARRAY + elem;
        }
        compile_error_parser(parser, "unknown generic type '%s'", type_name);
    }

    int pidx = parser_type_param_index(parser, type_name);
    if (pidx >= 0) {
        free(type_name);
        return MAKE_TYPE_PARAM(pidx);
    }

    int dt = type_to_type(type_name);
    if (dt != TYPE_UNKNOWN) {
        free(type_name);
        return dt;
    }
    int iid = interface_lookup_by_name(type_name);
    if (iid >= 0) {
        free(type_name);
        return MAKE_INTERFACE_TYPE(iid);
    }
    int cid = cust_lookup_by_name(type_name);
    if (cid >= 0) {
        free(type_name);
        return MAKE_CUST_TYPE(cid);
    }
    compile_error_parser(parser, "unknown type '%s'", type_name);
    free(type_name);
    return TYPE_UNKNOWN;
}

static AST_t *parse_cust_init_fields(parser_t *parser, int cust_id, const char *type_name)
{
    AST_t *init = parser_make_ast(parser, CUST_INIT_AST);
    init->int_value = cust_id;
    if (type_name)
        init->name = mkstr(type_name);
    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN) {
        if (parser->token->type != ID_TOKEN) {
            compile_error_parser(parser, "expected field name in cust initializer");
        }
        char *field = mkstr(parser->token->value);
        AST_t *entry = parser_make_ast(parser, ASSIGNEMENT_AST);
        entry->name = field;
        parser_next(parser, ID_TOKEN);
        parser_next(parser, EQUALS_TOKEN);
        AST_t *expr = parse_expression(parser);
        entry->parent = expr;
        list_enqueue(init->children, entry);
        if (parser->token->type == COMMA_TOKEN)
            parser_next(parser, COMMA_TOKEN);
    }
    parser_next(parser, RBRACE_TOKEN);
    return init;
}

static int parser_id_starts_method(parser_t *parser)
{
    if (parser->token->type != ID_TOKEN)
        return 0;
    lexer_t *lex = parser->lexer;
    int i = 0;
    while (isalnum((unsigned char)lexer_peek(lex, i)))
        i++;
    while (lexer_peek(lex, i) == ' ' || lexer_peek(lex, i) == '\t')
        i++;
    return lexer_peek(lex, i) == '=';
}

static int parser_peek_generic_cust_def(parser_t *parser)
{
    if (parser->token->type != LT_TOKEN)
        return 0;
    lexer_t *lex = parser->lexer;
    int i = 1;
    int depth = 1;
    while (depth > 0) {
        char c = lexer_peek(lex, i);
        if (!c)
            return 0;
        if (c == '<')
            depth++;
        else if (c == '>')
            depth--;
        i++;
    }
    while (lexer_peek(lex, i) == ' ' || lexer_peek(lex, i) == '\t')
        i++;
    if (lexer_peek(lex, i) != '=')
        return 0;
    i++;
    while (lexer_peek(lex, i) == ' ' || lexer_peek(lex, i) == '\t')
        i++;
    char kw[8];
    int k = 0;
    while (k < 7 && isalpha((unsigned char)lexer_peek(lex, i + k))) {
        kw[k] = lexer_peek(lex, i + k);
        k++;
    }
    kw[k] = '\0';
    return strcmp(kw, "cust") == 0 || strcmp(kw, "class") == 0;
}

static AST_t *parse_cust_method(parser_t *parser, const char *type_name, int visibility)
{
    int is_virtual = 0;
    if (parser->token->type == VIRTUAL_TOKEN) {
        parser_next(parser, VIRTUAL_TOKEN);
        is_virtual = 1;
    }

    char *method_name = mkstr(parser->token->value);
    parser_next(parser, ID_TOKEN);
    parser_next(parser, EQUALS_TOKEN);
    parser_next(parser, LPAREN_TOKEN);

    AST_t *func = parser_make_ast(parser, FUNC_AST);
    func->children = init_list(sizeof(struct AST_S *));

    if (parser->token->type == SELF_TOKEN ||
        (parser->token->type == ID_TOKEN && strcmp(parser->token->value, "self") == 0)) {
        parser_next(parser, parser->token->type);
        parser_next(parser, RPAREN_TOKEN);
        AST_t *self_param = parser_make_ast(parser, ASSIGNEMENT_AST);
        self_param->name = mkstr("self");
        self_param->datatype = TYPE_UNKNOWN;
        list_enqueue(func->children, self_param);
        func->id = 1;
    } else {
        compile_error_parser(parser, "methods in '%s' must declare (self) receiver", type_name);
    }

    if (parser->token->type != FUNCTION_TOKEN) {
        compile_error_parser(parser, "expected 'function' after method parameter list");
    }
    parser_next(parser, FUNCTION_TOKEN);
    func->parent = parse_compound(parser);
    func->parent->int_value = 0;
    if (parser->token->type == ID_TOKEN)
        func->datatype = parser_read_datatype(parser);

    func->name = cust_mangle_method(type_name, method_name);
    func->multiplier = visibility;
    func->int_value = -1;
    func->stack_index = is_virtual;
    free(method_name);
    return func;
}

static dynamic_list_t *parser_parse_type_param_names(parser_t *parser)
{
    parser_next(parser, LT_TOKEN);
    dynamic_list_t *names = init_list(sizeof(char *));
    if (parser->token->type != ID_TOKEN) {
        compile_error_parser(parser, "expected type parameter name");
    }
    list_enqueue(names, mkstr(parser->token->value));
    parser_next(parser, ID_TOKEN);
    while (parser->token->type == COMMA_TOKEN) {
        parser_next(parser, COMMA_TOKEN);
        if (parser->token->type != ID_TOKEN) {
            compile_error_parser(parser, "expected type parameter name");
        }
        list_enqueue(names, mkstr(parser->token->value));
        parser_next(parser, ID_TOKEN);
    }
    parser_next(parser, GT_TOKEN);
    return names;
}

static AST_t *parse_cust_body(parser_t *parser, char *type_name, int is_template)
{
    if (!parser_is_cust_or_class_token(parser)) {
        compile_error_parser(parser, "expected 'cust' or 'class' after type name");
    }
    parser_next(parser, parser->token->type);

    int base_type_id = -1;
    dynamic_list_t *iface_ids = init_list(sizeof(void *));

    if (parser->token->type == EXTENDS_TOKEN) {
        if (is_template) {
            compile_error_parser(parser, "generic types cannot use 'extends' in template definition");
        }
        parser_next(parser, EXTENDS_TOKEN);
        if (parser->token->type != ID_TOKEN) {
            compile_error_parser(parser, "expected base type name after 'extends'");
        }
        base_type_id = cust_lookup_by_name(parser->token->value);
        if (base_type_id < 0) {
            compile_error_parser(parser, "unknown base type '%s'", parser->token->value);
        }
        parser_next(parser, ID_TOKEN);
    }

    if (parser->token->type == IMPLEMENTS_TOKEN) {
        if (is_template) {
            compile_error_parser(parser, "generic templates cannot implement interfaces directly");
        }
        parser_next(parser, IMPLEMENTS_TOKEN);
        do {
            if (parser->token->type != ID_TOKEN) {
                compile_error_parser(parser, "expected interface name after 'implements'");
            }
            int iid = interface_lookup_by_name(parser->token->value);
            if (iid < 0) {
                compile_error_parser(parser, "unknown interface '%s'", parser->token->value);
            }
            list_enqueue(iface_ids, (void *)(intptr_t)iid);
            parser_next(parser, ID_TOKEN);
        } while (parser->token->type == COMMA_TOKEN && (parser_next(parser, COMMA_TOKEN), 1));
    }

    parser_next(parser, LBRACE_TOKEN);
    AST_t *def = parser_make_ast(parser, CUST_DEF_AST);
    def->name = type_name;

    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN) {
        int visibility = CUST_VIS_PUBLIC;
        if (parser->token->type == PUBLIC_TOKEN) {
            parser_next(parser, PUBLIC_TOKEN);
            visibility = CUST_VIS_PUBLIC;
        } else if (parser->token->type == PRIVATE_TOKEN) {
            parser_next(parser, PRIVATE_TOKEN);
            visibility = CUST_VIS_PRIVATE;
        }

        AST_t *member;
        if (parser->token->type == VIRTUAL_TOKEN || parser_id_starts_method(parser))
            member = parse_cust_method(parser, type_name, visibility);
        else {
            member = parse_factor(parser);
            member->multiplier = visibility;
        }
        list_enqueue(def->children, member);
        if (parser->token->type == SEMI_TOKEN)
            parser_next(parser, SEMI_TOKEN);
    }
    parser_next(parser, RBRACE_TOKEN);

    if (is_template) {
        generic_register_template(type_name, parser->generic_params, def->children, def);
        parser->generic_params = NULL;
        def->int_value = -1;
        list_free_shallow(iface_ids);
        return def;
    }

    int id = cust_register_from_ast(def, base_type_id, iface_ids);
    list_free_shallow(iface_ids);
    def->int_value = id;
    def->datatype = MAKE_CUST_TYPE(id);
    return def;
}

static AST_t *parse_generic_cust_def(parser_t *parser, char *type_name,
                                     dynamic_list_t *param_names)
{
    parser->generic_params = param_names;
    return parse_cust_body(parser, type_name, 1);
}

static AST_t *parse_cust_def(parser_t *parser, char *type_name)
{
    return parse_cust_body(parser, type_name, 0);
}

static AST_t *parse_interface_def(parser_t *parser, char *type_name)
{
    parser_next(parser, INTERFACE_TOKEN);
    parser_next(parser, LBRACE_TOKEN);
    AST_t *def = parser_make_ast(parser, CUST_DEF_AST);
    def->name = type_name;
    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN) {
        if (parser->token->type != ID_TOKEN && parser->token->type != VIRTUAL_TOKEN) {
            compile_error_parser(parser, "interface requires method signatures");
        }
        AST_t *member = parse_cust_method(parser, type_name, CUST_VIS_PUBLIC);
        list_enqueue(def->children, member);
        if (parser->token->type == SEMI_TOKEN)
            parser_next(parser, SEMI_TOKEN);
    }
    parser_next(parser, RBRACE_TOKEN);
    interface_register_from_ast(type_name, def->children, def);
    def->int_value = -2;
    return def;
}

AST_t * parse_type_literal(parser_t * parser)
{
    int dt = parser_read_datatype(parser);
    AST_t * ast = parser_make_ast(parser, TYPE_SIZE_AST);
    ast->datatype = dt;
    if (IS_CUST_TYPE(dt))
        ast->int_value = cust_heap_object_size(CUST_TYPE_ID(dt));
    else
        ast->int_value = datatype_heap_size(dt);
    return ast;
}

AST_t * parse_sizeof(parser_t * parser)
{
    parser_next(parser, ID_TOKEN);
    parser_next(parser, LPAREN_TOKEN);
    AST_t * ast = parse_type_literal(parser);
    parser_next(parser, RPAREN_TOKEN);
    return ast;
}

static AST_t * parse_rent_call(parser_t * parser, char * name)
{
    parser_next(parser, LPAREN_TOKEN);
    AST_t * list = parser_make_ast(parser, COMP_AST);
    list_enqueue(list->children, parse_expression(parser));
    if (parser->token->type == COMMA_TOKEN) {
        parser_next(parser, COMMA_TOKEN);
        list_enqueue(list->children, parse_type_literal(parser));
    }
    parser_next(parser, RPAREN_TOKEN);
    AST_t * ast = parser_make_ast(parser, CALL_AST);
    ast->name = name;
    ast->parent = list;
    return ast;
}

AST_t * parse_id(parser_t * parser) // this part mainly handles vairable declaration
{
    char * value  = mkstr(parser->token->value);
    parser_next(parser, ID_TOKEN);

    dynamic_list_t *type_params = NULL;
    if (parser->token->type == LT_TOKEN) {
        if (generic_lookup_template(value) >= 0) {
            int cid = parser_read_generic_instantiated(parser, value);
            if (parser->token->type == LBRACE_TOKEN) {
                parser_next(parser, LBRACE_TOKEN);
                cust_type_t *ct = cust_get(cid);
                return parse_cust_init_fields(parser, cid, ct && ct->name ? ct->name : value);
            }
            compile_error_parser(parser, "expected '{{' after generic type '%s<...>'", value);
        } else if (parser_peek_generic_cust_def(parser)) {
            type_params = parser_parse_type_param_names(parser);
        }
    }

    AST_t * ast = parser_make_ast(parser, VAR_AST);
    ast->name = value;

    if (strcmp(value, "HowBig") == 0 && parser->token->type == LPAREN_TOKEN) {
        parser_next(parser, LPAREN_TOKEN);
        AST_t *sz = parse_type_literal(parser);
        parser_next(parser, RPAREN_TOKEN);
        free(value);
        return sz;
    }

    // this is for varaible assignemnt
    if(parser->token->type == RBRACE_TOKEN)
    {
        parser_next(parser, RBRACE_TOKEN);
        if (parser_token_starts_datatype(parser)) {
            ast->datatype = parser_read_datatype(parser);
            ast->int_value = 1;
        }
    }

    else 
    {
        if (parser->token->type == LPAREN_TOKEN) // this is for function calls
        {
            if (strcmp(value, "rent") == 0)
                return parse_rent_call(parser, value);
            ast->type = CALL_AST;
            ast->parent = parse_list(parser);
        }
        else if (parser->token->type == LBRACE_TOKEN)
        {
            int cid = cust_lookup_by_name(value);
            if (cid >= 0) {
                parser_next(parser, LBRACE_TOKEN);
                return parse_cust_init_fields(parser, cid, value);
            }
        }
        else if (parser->token->type == LSQUAREBRKT_TOKEN)
        {
            parser_next(parser, LSQUAREBRKT_TOKEN);
            AST_t* start_expr = parse_expression(parser);
            if (parser->token->type == COLON_TOKEN)
            {
                parser_next(parser, COLON_TOKEN);
                AST_t* end_expr = parse_expression(parser);
                parser_next(parser, RSQUAREBRKT_TOKEN);
                AST_t* slice_ast = parser_make_ast(parser, SLICE_AST);
                list_enqueue(slice_ast->children, ast);
                list_enqueue(slice_ast->children, start_expr);
                list_enqueue(slice_ast->children, end_expr);
                return slice_ast;
            }
            else
            {
                parser_next(parser, RSQUAREBRKT_TOKEN);
                ast->type = ACCESS_AST;
                ast->left = start_expr;
            }
        }
    }

    AST_t *expr = ast;
    while (parser->token->type == DOT_TOKEN) {
        parser_next(parser, DOT_TOKEN);
        if (parser->token->type != ID_TOKEN) {
            compile_error_parser(parser, "expected field name after '.'");
        }
        char *fname = mkstr(parser->token->value);
        AST_t *fa = parser_make_ast(parser, FIELD_ACCESS_AST);
        fa->left = expr;
        fa->name = fname;
        parser_next(parser, ID_TOKEN);
        expr = fa;
        if (parser->token->type == LPAREN_TOKEN) {
            AST_t *call = parser_make_ast(parser, CALL_AST);
            call->left = fa->left;
            call->name = fname;
            call->parent = parser_make_ast(parser, COMP_AST);
            parser_next(parser, LPAREN_TOKEN);
            if (parser->token->type != RPAREN_TOKEN) {
                list_enqueue(call->parent->children, parse_expression(parser));
                while (parser->token->type == COMMA_TOKEN) {
                    parser_next(parser, COMMA_TOKEN);
                    list_enqueue(call->parent->children, parse_expression(parser));
                }
            }
            parser_next(parser, RPAREN_TOKEN);
            expr = call;
            break;
        }
    }
    ast = expr;

    if (parser->token->type == LSQUAREBRKT_TOKEN)
    {
        parser_next(parser, LSQUAREBRKT_TOKEN);
        AST_t *idx = parse_expression(parser);
        parser_next(parser, RSQUAREBRKT_TOKEN);
        AST_t *acc = parser_make_ast(parser, ACCESS_AST);
        acc->parent = ast;
        acc->left = idx;
        AST_t *root = ast;
        while (root->left && root->type == FIELD_ACCESS_AST)
            root = root->left;
        if (root->type == VAR_AST && root->name)
            acc->name = mkstr(root->name);
        ast = acc;
    }

    if (parser->token->type == EQUALS_TOKEN ||
        parser->token->type == PLUS_EQUALS_TOKEN ||
        parser->token->type == MINUS_EQUALS_TOKEN)
    {
        int op = parser->token->type;
        parser_next(parser, op);
        if (op == EQUALS_TOKEN && type_params && parser_is_cust_or_class_token(parser))
            return parse_generic_cust_def(parser, value, type_params);
        if (op == EQUALS_TOKEN && parser->token->type == INTERFACE_TOKEN)
            return parse_interface_def(parser, value);
        if (op == EQUALS_TOKEN && parser_is_cust_or_class_token(parser))
            return parse_cust_def(parser, value);

        if (ast->type == FIELD_ACCESS_AST) {
            AST_t *assign = parser_make_ast(parser, ASSIGNEMENT_AST);
            assign->op = op;
            assign->left = ast;
            assign->parent = parse_expression(parser);
            AST_t *base = ast;
            while (base->left && base->type == FIELD_ACCESS_AST)
                base = base->left;
            if (base->type == VAR_AST && base->name)
                assign->name = mkstr(base->name);
            return assign;
        }

        ast->op = op;
        ast->type = ASSIGNEMENT_AST;
        ast->name = value;

        ast->parent = parse_expression(parser);

        if (ast->op == EQUALS_TOKEN &&
            ast->parent->type != CALL_AST &&
            ast->parent->type != DUPE_AST &&
            ast->parent->type != ACCESS_AST &&
            ast->parent->type != SLICE_AST &&
            ast->parent->type != VAR_AST &&
            ast->parent->type != FIELD_ACCESS_AST &&
            ast->parent->type != CUST_INIT_AST)
        {
            ast->parent->name = mkstr(ast->name);
        }
    }

    return ast;


}

AST_t * parse_bool(parser_t * parser)
{
    AST_t * ast = parser_make_ast(parser, BOOL_AST);
    ast->int_value = (parser->token->type == REAL_TOKEN) ? 1 : 0;
    parser_next(parser, parser->token->type);
    return ast;
}

//This is the function used to parse factors
AST_t * parse_factor(parser_t * parser)
{
    switch (parser->token->type)
    {
        case INT_TOKEN:
            return parse_int(parser);
        case ID_TOKEN:
            if (strcmp(parser->token->value, "sizeof") == 0)
                return parse_sizeof(parser);
            return parse_id(parser);
        case STRING_TOKEN:
            return parse_string(parser);
        case REAL_TOKEN:
        case FAKE_TOKEN:
            return parse_bool(parser);
        case LPAREN_TOKEN:
            return parse_list(parser);
        case DUPE_TOKEN:
            return parse_dupe(parser);
        case LSQUAREBRKT_TOKEN:{
            parser_next(parser, LSQUAREBRKT_TOKEN);
            AST_t* arr = parser_make_ast(parser, ARRAY_LITERAL_AST);
            if (parser->token->type != RSQUAREBRKT_TOKEN) {
                AST_t* first = parse_expression(parser);
                if (parser->token->type == SEMI_TOKEN) {
                    AST_t* val = first;
                    while (1) {
                        parser_next(parser, SEMI_TOKEN);
                        AST_t* count_expr = parse_expression(parser);
                        if (count_expr->type != INT_AST) {
                            compile_error_parser(parser, "Array repeat count must be an integer literal");
                        }
                        int count = count_expr->int_value;
                        for (int i = 0; i < count; i++) {
                            AST_t* elem = parser_make_ast(parser, INT_AST);
                            elem->int_value = val->int_value;
                            list_enqueue(arr->children, elem);
                        }
                        if (parser->token->type == COMMA_TOKEN) {
                            parser_next(parser, COMMA_TOKEN);
                            val = parse_expression(parser);
                            if (parser->token->type != SEMI_TOKEN) {
                                compile_error_parser(parser, "Expected ';' after value in array range syntax");
                            }
                        } else {
                            break;
                        }
                    }
                } else {
                    list_enqueue(arr->children, first);
                    while (parser->token->type == COMMA_TOKEN) {
                        parser_next(parser, COMMA_TOKEN);
                        list_enqueue(arr->children, parse_expression(parser));
                    }
                }
            }
            parser_next(parser, RSQUAREBRKT_TOKEN);
            arr->int_value = arr->children->size;
            return arr;
        }
        case LBRACE_TOKEN:{
            lexer_t *lex = parser->lexer;
            if (isalpha((unsigned char)lex->c)) {
                int i = 1;
                while (isalnum((unsigned char)lexer_peek(lex, i)))
                    i++;
                if (lexer_peek(lex, i) == '=') {
                    parser_next(parser, LBRACE_TOKEN);
                    return parse_cust_init_fields(parser, -1, 0);
                }
            }
            parser_next(parser, LBRACE_TOKEN);
            return parse_id(parser);
        }
        default:
            compile_error_parser(parser, "Unexpected token '%s' (type %d)",
                                 parser->token->value, parser->token->type);
    }
}

// postfix: primary then optional ++/--
AST_t * parse_postfix(parser_t * parser)
{
    AST_t * primary = parse_factor(parser);
    while (parser->token->type == PLUS_PLUS_TOKEN || parser->token->type == MINUS_MINUS_TOKEN) {
        AST_t * incdec = parser_make_ast(parser, INC_DEC_AST);
        incdec->left = primary;
        incdec->op = parser->token->type;
        incdec->int_value = 1; /* postfix */
        parser_next(parser, parser->token->type);
        primary = incdec;
    }
    return primary;
}

// unary: prefix ++/--, not, ~
AST_t * parse_unary(parser_t * parser)
{
    if (parser->token->type == PLUS_PLUS_TOKEN || parser->token->type == MINUS_MINUS_TOKEN) {
        AST_t * incdec = parser_make_ast(parser, INC_DEC_AST);
        incdec->op = parser->token->type;
        parser_next(parser, parser->token->type);
        incdec->left = parse_unary(parser);
        incdec->int_value = 0; /* prefix */
        return incdec;
    }
    if (parser->token->type == NOT_TOKEN || parser->token->type == BITNOT_TOKEN) {
        AST_t * unary = parser_make_ast(parser, UNARY_AST);
        unary->op = parser->token->type;
        parser_next(parser, parser->token->type);
        unary->left = parse_unary(parser);
        return unary;
    }
    return parse_postfix(parser);
}

// multiplication, division
AST_t * parse_term(parser_t *parser)
{
    AST_t * left = parse_unary(parser);

    while (parser->token->type == ASTERISK_TOKEN || parser->token->type == SLASH_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_unary(parser);
        left = binop;
    }

    return left;
}

// addition, subtraction
AST_t * parse_addition(parser_t* parser)
{
    AST_t * left = parse_term(parser);

    while (parser->token->type == PLUS_TOKEN || parser->token->type == MINUS_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_term(parser);
        left = binop;
    }
    return left;
}

// bitwise and
AST_t * parse_bitand(parser_t* parser)
{
    AST_t * left = parse_addition(parser);

    while (parser->token->type == BITAND_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_addition(parser);
        left = binop;
    }
    return left;
}

// bitwise or
AST_t * parse_bitor(parser_t* parser)
{
    AST_t * left = parse_bitand(parser);

    while (parser->token->type == BITOR_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_bitand(parser);
        left = binop;
    }
    return left;
}

// comparisons: ==  !=  <  >  <=  >=
AST_t * parse_comparison(parser_t* parser)
{
    AST_t * left = parse_bitor(parser);

    while (parser->token->type == DEQUALS_TOKEN ||
           parser->token->type == NOT_EQUALS_TOKEN ||
           parser->token->type == LT_TOKEN ||
           parser->token->type == GT_TOKEN ||
           parser->token->type == LTE_TOKEN ||
           parser->token->type == GTE_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_bitor(parser);
        left = binop;
    }
    return left;
}

// logical and
AST_t * parse_and(parser_t* parser)
{
    AST_t * left = parse_comparison(parser);

    while (parser->token->type == AND_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_comparison(parser);
        left = binop;
    }
    return left;
}

// logical or  (lowest precedence)
AST_t * parse_expression(parser_t* parser)
{
    AST_t * left = parse_and(parser);

    while (parser->token->type == OR_TOKEN)
    {
        AST_t * binop = parser_make_ast(parser, BINOP_AST);
        binop->left = left;
        binop->op = parser->token->type;
        parser_next(parser, parser->token->type);
        binop->right = parse_and(parser);
        left = binop;
    }
    return left;
}


AST_t * parse_loop_until(parser_t * parser);

AST_t * parse_dupe(parser_t * parser)
{
    parser_next(parser, DUPE_TOKEN);
    parser_next(parser, LPAREN_TOKEN);

    if (parser->token->type != ID_TOKEN) {
        compile_error_parser(parser, "dupe() expects a function name as first argument");
    }
    char * fn = mkstr(parser->token->value);
    parser_next(parser, ID_TOKEN);

    parser_next(parser, COMMA_TOKEN);
    AST_t * arg = parse_expression(parser);
    parser_next(parser, RPAREN_TOKEN);

    AST_t * node = parser_make_ast(parser, DUPE_AST);
    node->name = fn;
    node->left = arg;
    return node;
}

//This is the function used to parse return statements
AST_t * parse_statement(parser_t * parser)
{
    AST_t * ast = parser_make_ast(parser, RETURN_AST);
    parser_next(parser, RETURN_TOKEN);
    ast->parent = parse_expression(parser);
    return ast;
}

/*
 * if (condition) { body } else if (condition) { body } else { body };
 * IF_AST: left=condition, children=body, right=next else/else-if (IF_AST or NULL)
 */
AST_t * parse_if(parser_t * parser)
{
    parser_next(parser, IF_TOKEN);
    AST_t * node = parser_make_ast(parser, IF_AST);

    parser_next(parser, LPAREN_TOKEN);
    node->left = parse_expression(parser);
    parser_next(parser, RPAREN_TOKEN);

    parser_next(parser, LBRACE_TOKEN);
    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
    {
        if (parser->token->type == RETURN_TOKEN)
            list_enqueue(node->children, parse_statement(parser));
        else if (parser->token->type == IF_TOKEN)
            list_enqueue(node->children, parse_if(parser));
        else if (parser->token->type == DUPE_TOKEN)
            list_enqueue(node->children, parse_dupe(parser));
        else if (parser->token->type == LBRACE_TOKEN && !parser_brace_starts_var_decl(parser))
            list_enqueue(node->children, parse_compound(parser));
        else
            list_enqueue(node->children, parse_expression(parser));
        if (parser->token->type == SEMI_TOKEN)
            parser_next(parser, SEMI_TOKEN);
    }
    parser_next(parser, RBRACE_TOKEN);

    if (parser->token->type == ELSE_TOKEN)
    {
        parser_next(parser, ELSE_TOKEN);
        if (parser->token->type == IF_TOKEN)
        {
            node->right = parse_if(parser);
        }
        else
        {
            AST_t * else_node = parser_make_ast(parser, IF_AST);
            else_node->left = NULL;
            parser_next(parser, LBRACE_TOKEN);
            while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
            {
                if (parser->token->type == RETURN_TOKEN)
                    list_enqueue(else_node->children, parse_statement(parser));
                else if (parser->token->type == IF_TOKEN)
                    list_enqueue(else_node->children, parse_if(parser));
                else if (parser->token->type == DUPE_TOKEN)
                    list_enqueue(else_node->children, parse_dupe(parser));
                else if (parser->token->type == LBRACE_TOKEN && !parser_brace_starts_var_decl(parser))
                    list_enqueue(else_node->children, parse_compound(parser));
                else
                    list_enqueue(else_node->children, parse_expression(parser));
                if (parser->token->type == SEMI_TOKEN)
                    parser_next(parser, SEMI_TOKEN);
            }
            parser_next(parser, RBRACE_TOKEN);
            node->right = else_node;
        }
    }

    return node;
}

/* Parse loop body (statements until RBRACE). Consumes RBRACE. */
static void parse_loop_body(parser_t *p, AST_t *n)
{
    while (p->token->type != RBRACE_TOKEN && p->token->type != EOF_TOKEN) {
        if (p->token->type == RETURN_TOKEN)
            list_enqueue(n->children, parse_statement(p));
        else if (p->token->type == IF_TOKEN)
            list_enqueue(n->children, parse_if(p));
        else if (p->token->type == LOOP_TOKEN)
            list_enqueue(n->children, parse_loop_until(p));
        else if (p->token->type == DUPE_TOKEN)
            list_enqueue(n->children, parse_dupe(p));
        else if (p->token->type == LBRACE_TOKEN && !parser_brace_starts_var_decl(p))
            list_enqueue(n->children, parse_compound(p));
        else
            list_enqueue(n->children, parse_expression(p));
        if (p->token->type == SEMI_TOKEN)
            parser_next(p, SEMI_TOKEN);
    }
    parser_next(p, RBRACE_TOKEN);
}

/*
 * loop until (condition) { body };     -> while
 * loop until (init; cond; step) { body }; -> for
 * loop { body } until (condition);     -> do-while
 * loop { body } until (init; cond; step); -> do-while for-style
 * LOOP_UNTIL_AST: left = condition (expr or FOR_CLAUSE_AST), children = body, int_value = body_first (0 or 1)
 */
AST_t * parse_loop_until(parser_t * parser)
{
    parser_next(parser, LOOP_TOKEN);
    AST_t * node = parser_make_ast(parser, LOOP_UNTIL_AST);
    int body_first = 0;

    if (parser->token->type == UNTIL_TOKEN) {
        parser_next(parser, UNTIL_TOKEN);
        parser_next(parser, LPAREN_TOKEN);
        AST_t * first = parse_expression(parser);
        if (parser->token->type == SEMI_TOKEN) {
            AST_t * for_clause = parser_make_ast(parser, FOR_CLAUSE_AST);
            list_enqueue(for_clause->children, first);
            parser_next(parser, SEMI_TOKEN);
            list_enqueue(for_clause->children, parse_expression(parser));
            parser_next(parser, SEMI_TOKEN);
            list_enqueue(for_clause->children, parse_expression(parser));
            parser_next(parser, RPAREN_TOKEN);
            node->left = for_clause;
        } else {
            parser_next(parser, RPAREN_TOKEN);
            node->left = first;
        }
        parser_next(parser, LBRACE_TOKEN);
        parse_loop_body(parser, node);
    } else if (parser->token->type == LBRACE_TOKEN) {
        body_first = 1;
        parser_next(parser, LBRACE_TOKEN);
        parse_loop_body(parser, node);
        parser_next(parser, UNTIL_TOKEN);
        parser_next(parser, LPAREN_TOKEN);
        AST_t * first = parse_expression(parser);
        if (parser->token->type == SEMI_TOKEN) {
            AST_t * for_clause = parser_make_ast(parser, FOR_CLAUSE_AST);
            list_enqueue(for_clause->children, first);
            parser_next(parser, SEMI_TOKEN);
            list_enqueue(for_clause->children, parse_expression(parser));
            parser_next(parser, SEMI_TOKEN);
            list_enqueue(for_clause->children, parse_expression(parser));
            parser_next(parser, RPAREN_TOKEN);
            node->left = for_clause;
        } else {
            parser_next(parser, RPAREN_TOKEN);
            node->left = first;
        }
    } else {
        compile_error_parser(parser, "expected 'until' or '{' after 'loop'");
    }
    node->int_value = body_first;
    return node;
}

//This is the function used to parse lists
AST_t * parse_list(parser_t * parser)
{
    // printf("[List] Current Token : %s\n", parser->token->value);
    bool is_bracket = parser->token->type == LSQUAREBRKT_TOKEN;
    bool is_brace = parser->token->type == LBRACE_TOKEN;

    if (!is_brace)
    {
        parser_next(parser, is_bracket ? LSQUAREBRKT_TOKEN : LPAREN_TOKEN);
    }

    AST_t * list = parser_make_ast(parser, COMP_AST);

    if(parser->token->type != (is_bracket ? RSQUAREBRKT_TOKEN : RPAREN_TOKEN))
    {
        list_enqueue(list->children, parse_expression(parser));

        while (parser->token->type == COMMA_TOKEN)
        {
            parser_next(parser, COMMA_TOKEN);
            list_enqueue(list->children, parse_expression(parser));
        }
    }

    if(!is_brace)
    {
        parser_next(parser, is_bracket ? RSQUAREBRKT_TOKEN : RPAREN_TOKEN);
    }


    if (parser->token->type == RBRACE_TOKEN)
    {
        parser_next(parser, RBRACE_TOKEN);
        while(parser->token->type == ID_TOKEN)
        {
            list->datatype = parser_read_datatype(parser);
        }
    }

    if(parser->token->type == FUNCTION_TOKEN)
    {
        parser_next(parser, FUNCTION_TOKEN);
        list->type = FUNC_AST;
        list->parent = parse_compound(parser);
        list->parent->int_value = 0;

        /* Consume the return type after function body (e.g., } int;) */
        if (parser->token->type == ID_TOKEN)
        {
            list->datatype = parser_read_datatype(parser);
        }

        for (int i = 0; i < list->children->size; i++)
        {
            ((AST_t*)list->children->items[i])->type = ASSIGNEMENT_AST;
        }

    }

    return list;
        
}


//This is the function used to parse strings
AST_t * parse_string(parser_t * parser)
{
    char * value = mkstr(parser->token->value);
    AST_t * ast = parser_make_ast(parser, STRING_AST);
    ast->string_value = value;
    parser_next(parser, STRING_TOKEN);
    return ast;
}

//This is the function used to create a "compound" or pretty much a block of code with {}
AST_t * parse_compound(parser_t * parser)
{
    bool is_braced = false;


    if (parser->token->type == LBRACE_TOKEN)
    {
        is_braced = true;
        parser_next(parser, LBRACE_TOKEN);
    }

    AST_t * compound = parser_make_ast(parser, COMP_AST);

    while (parser->token->type != RBRACE_TOKEN && parser->token->type != EOF_TOKEN)
    {
        if (parser->token->type == RETURN_TOKEN)
        {
            list_enqueue(compound->children, parse_statement(parser));
        }
        else if (parser->token->type == IF_TOKEN)
        {
            list_enqueue(compound->children, parse_if(parser));
        }
        else if (parser->token->type == LOOP_TOKEN)
        {
            list_enqueue(compound->children, parse_loop_until(parser));
        }
        else if (parser->token->type == DUPE_TOKEN)
        {
            list_enqueue(compound->children, parse_dupe(parser));
        }
        else if (parser->token->type == LBRACE_TOKEN && !parser_brace_starts_var_decl(parser))
        {
            list_enqueue(compound->children, parse_compound(parser));
        }
        else 
        {
            list_enqueue(compound->children, parse_expression(parser));
        }
        if (parser->token->type == SEMI_TOKEN)
        {
            parser_next(parser, SEMI_TOKEN);
        }
    }

    if(is_braced)
    {
        parser_next(parser, RBRACE_TOKEN);
        compound->int_value = 1;
    }

    return compound;

}



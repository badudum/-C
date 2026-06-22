#include "../include/constexpr.h"
#include "../include/token.h"
#include <stdlib.h>
#include <string.h>

static int ce_int_from_node(AST_t *node, dynamic_list_t *list, int *out)
{
    if (!node || !out)
        return 0;
    if (node->type == INT_AST) {
        *out = node->int_value;
        return 1;
    }
    if (node->type == TYPE_SIZE_AST && node->int_value > 0) {
        *out = node->int_value;
        return 1;
    }
    if (node->type == VAR_AST && node->name && list) {
        for (int j = (int)list->size - 1; j >= 0; j--) {
            AST_t *def = (AST_t *)list->items[j];
            if (def->type == ASSIGNEMENT_AST && def->name &&
                strcmp(def->name, node->name) == 0 && def->parent &&
                def->parent->type == INT_AST) {
                *out = def->parent->int_value;
                return 1;
            }
        }
    }
    return 0;
}

int constexpr_eval_int(AST_t *node, dynamic_list_t *list, int *out)
{
    if (!node || !out)
        return 0;

    if (ce_int_from_node(node, list, out))
        return 1;

    if (node->type == UNARY_AST && node->op == MINUS_TOKEN && node->left) {
        int v;
        if (constexpr_eval_int(node->left, list, &v)) {
            *out = -v;
            return 1;
        }
        return 0;
    }

    if (node->type != BINOP_AST || !node->left || !node->right)
        return 0;

    int a, b;
    if (!constexpr_eval_int(node->left, list, &a) ||
        !constexpr_eval_int(node->right, list, &b))
        return 0;

    switch (node->op) {
    case PLUS_TOKEN:  *out = a + b; return 1;
    case MINUS_TOKEN: *out = a - b; return 1;
    case ASTERISK_TOKEN:
        *out = a * b;
        return 1;
    case SLASH_TOKEN:
        if (b == 0)
            return 0;
        *out = a / b;
        return 1;
    case MODULUS_TOKEN:
        if (b == 0)
            return 0;
        *out = a % b;
        return 1;
    default:
        return 0;
    }
}

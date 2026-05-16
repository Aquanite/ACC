/*
Aquanite Compiler Collection – Copyright (C) 2026
*/

#include <libstellar/stellar.h>
#include <token.h>


token_list_t* new_token_list(void) {
    token_list_t* list = (token_list_t*)malloc(sizeof(token_list_t));
    list->next = NULL;
    list->prev = NULL;
    return list;
}

void add_token_to_list(token_list_t** list, token_t token) {
    NULLCHECK(list);
    NULLCHECK(*list);

    token_list_t* node = *list;

    while (node->next != NULL)
        node = node->next;

    token_list_t* new_node = malloc(sizeof(token_list_t));
    NULLCHECK(new_node);

    new_node->token = token;
    new_node->next = NULL;
    new_node->prev = node;
    node->next = new_node;
}

token_t peek_token(token_list_t** list) {
    if (ISNULL(list) || ISNULL(*list))
        return (token_t) {.type = TOK_EOF};

    return (*list)->token;
}

token_t fetch_token(token_list_t** list) {
    if (ISNULL(list) || ISNULL(*list))
        return (token_t) {.type = TOK_EOF};

    token_list_t* node_to_free = *list;
    token_t tok = node_to_free->token;

    *list = node_to_free->next;
    if (!ISNULL(*list)) {
        (*list)->prev = NULL;
    }

    free(node_to_free);
    return tok;
}


void free_token_list(token_list_t** list) {
    if (ISNULL(list) || ISNULL(*list))
        return;

    printf("Freed\n");

    token_list_t* current = *list;
    token_list_t* next_node;

    while (!ISNULL(current)) {
        next_node = current->next;
        free(current);
        current = next_node;
    }
    *list = NULL;
}

string token_to_name(token_type_t type) {
    switch (type) {
        case TOK_UNKNOWN: return "<unknown>";
        case TOK_INT: return "int";
        case TOK_VOID: return "void";
        case TOK_RETURN: return "return";
        case TOK_IDENTIFIER: return "identifier";
        case TOK_LPAREN: return "(";
        case TOK_RPAREN: return ")";
        case TOK_LCURLY: return "{";
        case TOK_RCURLY: return "}";
        case TOK_SCOLON: return ";";
        case TOK_UNARY_MINUS: return "- (unary)";
        case TOK_LITERAL: return "literal";
        case TOK_STRING: return "string literal";
        case TOK_EOF: return "<EOF>";
        default: return "<invalid token>";
    }
}

bool token_is_type(token_type_t type) {
    if (type == TOK_INT  ||
        type == TOK_VOID
    ) return true;

    return false;
}

bool token_is_control(token_type_t type) {
    if (type == TOK_RETURN
    ) return true;

    return false;
}

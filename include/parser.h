#ifndef ACC_PARSER_H
#define ACC_PARSER_H

#include <libstellar/stellar.h>
#include <token.h>
#include <lexer.h>

struct _function_t;

enum_t _expr_type_t {
    EXPR_LITERAL,
    EXPR_IDENTIFIER,
    EXPR_BINOP,
    EXPR_UNOP,
} expr_type_t;

enum_t _binop_t {
    BINOP_SUB,
} binop_t;

enum_t _unop_t {
    UNOP_NEG,
} unop_t;

struct_t _expr_t {
    expr_type_t type;
    union {
        int64_t literal;
        string identifier;
        struct {
            binop_t op;
            struct _expr_t* left;
            struct _expr_t* right;
        } binop;
        struct {
            unop_t op;
            struct _expr_t* operand;
        } unop;
    };
} expr_t;

enum_t _statement_type_t {
    STATE_RETURN,
    STATE_SCOPE_DECL,
} statement_type_t;

struct_t {
    bool is_void;
    expr_t* expr;
} state_return_t;

struct_t _statement_t {
    statement_type_t type;
    union {
        state_return_t return_state;
    };
} statement_t;

struct_t _body_t {
    struct _function_t* parent;
    statement_t* statments;
    size_t statement_count;
} body_t;

struct_t _function_t {
    string name;
    token_type_t return_type;
    body_t body;

    size_t line;
    size_t column;
} function_t;

struct_t _parser_t {
    string src;
    token_list_t* tokens;
    lexer_t* lexer;
    function_t* functions;
    size_t function_count;
} parser_t;

parser_t* new_parser(token_list_t** tokens, lexer_t** lexer);
parser_t* run_parser(parser_t* parser);

#endif // ACC_PARSER_H

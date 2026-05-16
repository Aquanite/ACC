#ifndef ACC_TOKEN_H
#define ACC_TOKEN_H

#include <libstellar/stellar.h>

enum_t {
    TOK_UNKNOWN,
    TOK_INT,
    TOK_VOID,
    TOK_RETURN,
    TOK_IDENTIFIER,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LCURLY,
    TOK_RCURLY,
    TOK_SCOLON,
    TOK_UNARY_MINUS,
    TOK_LITERAL,
    TOK_STRING,
    TOK_EOF,
} token_type_t;

struct_t {
    token_type_t type;
    
    string raw;

    union {
        string identifier;
        long long integer;
    };

    size_t line;
    size_t column;
} token_t;


struct_t _token_list_t {
    token_t token;
    struct _token_list_t* next;
    struct _token_list_t* prev;
} token_list_t;

token_t peek_token(token_list_t** list);
token_t fetch_token(token_list_t** list);
token_list_t* new_token_list(void);
void add_token_to_list(token_list_t** list, token_t token);
void free_token_list(token_list_t** list);
string token_to_name(token_type_t type);
bool token_is_type(token_type_t type);
bool token_is_control(token_type_t type);

#endif // ACC_TOKEN_H

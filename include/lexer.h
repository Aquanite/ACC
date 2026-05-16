#ifndef ACC_LEXER_H
#define ACC_LEXER_H

#include <token.h>

struct_t {
    string source;
    string file;
    token_list_t* tokens;
    size_t line;
    size_t column;
} lexer_t;

lexer_t* new_lexer(string _source, string _file);
token_list_t* run_lexer(lexer_t* lexer);

#endif // ACC_LEXER_H

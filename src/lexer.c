#include <lexer.h>
#include <libstellar/stellar.h>
#include <ctype.h>

#define IDENTCMP(w, x, y) (y == sizeof(x)-1 && strncmp(w,x,sizeof(x)-1) == 0)
#define ISIDENTSTART(c) (isalpha((unsigned char)(c)) || (c) == '_')
#define ISIDENT(c) (isalnum((unsigned char)(c)) || (c) == '_')

void lexer_assert(lexer_t* lex, int condition, string str, size_t line, size_t column, string item) {
    if (condition) 
        return;

    fprintf(stderr, 
            "%s:%zu:%zu: \033[1;31merror:\033[0m %s\n"
            "  %zu  |  %s\n", 
            lex->file, line, column, str,
            line, item);

    int digits = COUNTDIGITSU(line) + 4; // Account for spaces
    for (int i = 0; i < digits; i++)
        putc(' ', stderr);
    
    fprintf(stderr, "|  \033[1;31m^");

    size_t len = strlen(item) - 1;

    for (size_t i = 0; i < len; i++)
        putc('~', stderr);

    puts("\033[0m");
    
    exit(-1); 
}

lexer_t* new_lexer(string _source, string _file) {
    NULLCHECK(_source);
    NULLCHECK(_file);

    lexer_t* lexer = (lexer_t*)malloc(sizeof(lexer_t));
    NULLCHECK(lexer);

    lexer->source = _source;
    lexer->file   = _file;
    lexer->tokens = new_token_list();
    lexer->column = 1;
    lexer->line   = 1;

    return lexer;
}

token_list_t* run_lexer(lexer_t* lexer) {
    NULLCHECK(lexer);
    NULLCHECK(lexer->source);
    NULLCHECK(lexer->file);
    NULLCHECK(lexer->tokens);

    string src = lexer->source;

    size_t line = 1;
    size_t column = 1;
    
    while (*src) {
        char c = *src;

        if (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r') {
            column++;
            src++;
            continue;
        } else if (c == '\n') {
            line++;
            column = 1;
            src++;
            continue;
        }

        if (ISIDENTSTART(*src)) {
            const string start = src;
            size_t original_column = column;

            while(ISIDENT(*src)) {
                src++;
                column++;
            }

            size_t len = src - start;
            token_t tok = { .line = line, .column = original_column, .raw = strndup(start, len) };

            if (IDENTCMP(start, "int", len))         tok.type = TOK_INT;
            else if (IDENTCMP(start, "return", len)) tok.type = TOK_RETURN;
            else if (IDENTCMP(start, "void", len))   tok.type = TOK_VOID;
            else { // Nothing, since it has been valid until now, mark as identifier
                tok.type       = TOK_IDENTIFIER;
                tok.identifier = strndup(start, len);
            }

            add_token_to_list(&lexer->tokens, tok);
        } else if (isdigit((unsigned char)*src) || *src == '-') {
            const string start = src;
            size_t original_column = column;

            if (*src == '-') {
                src++;
                column++;

                if (!isdigit((unsigned char)*src)) {
                    add_token_to_list(&lexer->tokens, (token_t) {
                        .type = TOK_UNARY_MINUS,
                        .line = line,
                        .column = column,
                        .raw = "-"
                    });
                    continue;
                }
            }

            while(isdigit((unsigned char)*src)) {
                src++;
                column++;
            }

            size_t len = src - start;
            string copy = strndup(start, len);

            NULLCHECK(copy);

            string endPtr;

            long long value = strtoll(copy, &endPtr, 10);
            
            lexer_assert(lexer, 
                         copy != endPtr, 
                         "value is not valid integer", 
                         line, 
                         original_column, 
                         copy);

            add_token_to_list(&lexer->tokens, (token_t) {
                .type = TOK_LITERAL,
                .integer = value,
                .line = line, 
                .column = original_column,
                .raw = copy
            });
        } else if (ispunct((unsigned char)*src)) {
            if (*src == '(') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_LPAREN,
                    .line = line,
                    .column = column,
                    .raw = "("
                });
            } else if (*src == ')') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_RPAREN,
                    .line = line,
                    .column = column,
                    .raw = ")"
                });
            } else if (*src == '{') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_LCURLY,
                    .line = line,
                    .column = column,
                    .raw = "{"
                });
            } else if (*src == '}') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_RCURLY,
                    .line = line,
                    .column = column,
                    .raw = "}"
                });
            } else if (*src == ';') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_SCOLON,
                    .line = line,
                    .column = column,
                    .raw = ";"
                });
            } else if (*src == '-') {
                add_token_to_list(&lexer->tokens, (token_t) {
                    .type = TOK_UNARY_MINUS,
                    .line = line,
                    .column = column,
                    .raw = "-"
                });
            } else {
                lexer_assert(lexer, 
                             0, 
                             "unsupported punctuation", 
                             line, 
                             column, 
                             strndup(src, 1));
            }

            column++;
            src++;
        } else {
            lexer_assert(lexer, 
                         0, 
                         "unknown value in lexing", 
                         line, 
                         column, 
                         strndup(src, 1));
        }
    }

    add_token_to_list(&lexer->tokens, (token_t) {.type = TOK_EOF, .column = column, .line = line, .raw = "<EOF>"});

    return lexer->tokens;
}

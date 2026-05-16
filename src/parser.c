#include <parser.h>

static void dismantle(parser_t* parser, expr_t* expr);

void parser_assert(parser_t* parse, int condition, string str, size_t line, size_t column, string item) {
    if (condition) 
        return;

    fprintf(stderr, 
            "%s:%zu:%zu: \033[1;31merror:\033[0m %s\n"
            "  %zu  |  %s\n", 
            parse->lexer->file, line, column, str,
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

static expr_t* formulate_literal(int64_t val) {
    expr_t* expr = malloc(sizeof(expr_t));
    NULLCHECK(expr);

    expr->type = EXPR_LITERAL;
    expr->literal = val;
    return expr;
}

static expr_t* formulate_identifier(string ident) {
    expr_t* expr = malloc(sizeof(expr_t));
    NULLCHECK(expr);

    expr->type = EXPR_IDENTIFIER;
    expr->identifier = ident;
    return expr;
}

static expr_t* formulate_binop(binop_t op, expr_t* l, expr_t* r) {
    expr_t* expr = malloc(sizeof(expr_t));
    NULLCHECK(expr);

    expr->type = EXPR_BINOP;
    expr->binop.op = op;
    expr->binop.left = l;
    expr->binop.right = r;
    return expr;
}

static void dismantle(parser_t* parser, expr_t* expr) {
    if (expr == NULL) return;

    switch (expr->type) {
        case EXPR_LITERAL: free(expr); return;
        case EXPR_IDENTIFIER: free(expr); return;
        case EXPR_BINOP: {
            dismantle(parser, expr->binop.left);
            dismantle(parser, expr->binop.right);

            free(expr);
            return;
        }
        case EXPR_UNOP: {
            dismantle(parser, expr->unop.operand);

            free(expr);
            return;
        }
    }

    parser_assert(parser, 0, "unknown dismantle expression.", 0, 0, "");
}

static token_t parser_peek(parser_t* parser) {
    return peek_token(&parser->tokens);
}

static token_t parser_pop(parser_t* parser) {
    return fetch_token(&parser->tokens);
}

static bool parser_check(parser_t* parser, token_type_t type) {
    return parser_peek(parser).type == type;
}

static bool parser_is_type(parser_t* parser) {
    return token_is_type(parser_peek(parser).type);
}

static bool parser_is_control(parser_t* parser) {
    return token_is_control(parser_peek(parser).type);
}

static void parser_expect(parser_t* parser, token_type_t type) {
    token_t actual = parser_peek(parser);

    size_t len = strlen(token_to_name(type))   + 
                 strlen(token_to_name(actual.type)) + 
                 sizeof("expected token \"\" got \"\"");

    string buffer = malloc(len);

    snprintf(
        buffer, 
        len, 
        "expected token \"%s\" got \"%s\"", 
        token_to_name(type), 
        token_to_name(actual.type)
    );

    parser_assert(
        parser, 
        actual.type == type, 
        buffer, 
        parser_peek(parser).line, 
        parser_peek(parser).column, 
        actual.raw
    );

    free(buffer);
}

static void parser_body_append_statement(body_t* body, statement_t statement) {
    size_t next_count = body->statement_count + 1;
    statement_t* statements = realloc(body->statments, sizeof(statement_t) * next_count);
    NULLCHECK(statements);

    body->statments = statements;
    body->statments[body->statement_count] = statement;
    body->statement_count = next_count;
}

static token_type_t parser_literal_to_type(token_t token) {
    if (token.type == TOK_LITERAL) {
        return TOK_INT;
    }

    return TOK_UNKNOWN;
}

static statement_t parser_parse_return_statement(parser_t* parser, function_t* function) {
    statement_t statement = { .type = STATE_RETURN };

    parser_pop(parser); // drop return

    if (function->return_type == TOK_VOID) {
        parser_expect(parser, TOK_SCOLON);
        parser_pop(parser);

        statement.return_state.is_void = true;
        statement.return_state.expr = NULL;
        
        return statement;
    }

    parser_expect(parser, TOK_LITERAL);

    token_t value = parser_pop(parser);

    statement.return_state.type = value.type;
    statement.return_state.int_type = value.integer;

    parser_assert(
        parser,
        function->return_type == parser_literal_to_type(value),
        "return type does not match the enclosing function.",
        value.line,
        value.column,
        value.raw
    );

    parser_expect(parser, TOK_SCOLON); 
    parser_pop(parser);

    return statement;
}

static statement_t parser_parse_statement(parser_t* parser, function_t* function) {
    if (parser_check(parser, TOK_RETURN)) {
        return parser_parse_return_statement(parser, function);
    }

    parser_assert(parser, 0, "expected statement inside function body.", parser_peek(parser).line, parser_peek(parser).column, parser_peek(parser).raw);

    return (statement_t) { 0 };
}

static void parser_parse_function_body(parser_t* parser, function_t* function) {
    while (!parser_check(parser, TOK_RCURLY) && !parser_check(parser, TOK_EOF)) {
        parser_body_append_statement(&function->body, parser_parse_statement(parser, function));
    }

    parser_expect(parser, TOK_RCURLY);
    parser_pop(parser);
}

static function_t* parser_parse_function(parser_t* parser, token_t type, token_t ident) {
    function_t* func = malloc(sizeof(function_t));
    NULLCHECK(func);

    func->name        = ident.identifier;
    func->return_type = type.type;
    func->line        = type.line;
    func->column      = type.column;

    body_t body = { .parent = func, .statments = NULL, .statement_count = 0 };

    func->body = body;

    parser->function_count++;

    parser_pop(parser); // drop (

    /// TODO: Actual args parsing

    parser_expect(parser, TOK_RPAREN);
    parser_pop(parser);

    /// TODO: Function Definition

    parser_expect(parser, TOK_LCURLY);
    parser_pop(parser);

    parser_parse_function_body(parser, func);

    return func;
}

parser_t* new_parser(token_list_t** tokens, lexer_t** lexer) {
    NULLCHECK(tokens);
    NULLCHECK(lexer);
    NULLCHECK(*tokens);
    NULLCHECK(*lexer);
    NULLCHECK((*lexer)->source);

    parser_t* parser = (parser_t*)malloc(sizeof(parser_t));
    NULLCHECK(parser);

    parser->lexer = *lexer;
    parser->tokens = *tokens;
    parser->src = (*lexer)->source;
    parser->functions = NULL;

    return parser;
}

parser_t* run_parser(parser_t* parser) {
    while (!parser_check(parser, TOK_EOF)) {
        if (parser_is_type(parser)) {
            token_t type = parser_pop(parser);

            parser_expect(parser, TOK_IDENTIFIER); // function/var name

            token_t ident = parser_pop(parser);

            if (parser_check(parser, TOK_LPAREN)) { // function
                parser->functions = parser_parse_function(parser, type, ident);
                continue;
            }

            parser_assert(parser, 0, "expected function declaration.", parser_peek(parser).line, parser_peek(parser).column, parser_peek(parser).raw);
        } else if (parser_is_control(parser)) {
            parser_assert(parser, 0, "unexpected control word outside of function declaration.", parser_peek(parser).line, parser_peek(parser).column, parser_peek(parser).raw);
        }

        parser_assert(parser, 0, "unexpected token in parser.", parser_peek(parser).line, parser_peek(parser).column, parser_peek(parser).raw);
    }
    
    return parser;
}

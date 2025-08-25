// src/parser.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Create parser
parser_t* parser_create(lexer_t* lexer) {
    if (!lexer) return NULL;
    
    parser_t* parser = malloc(sizeof(parser_t));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    parser->error_count = 0;
    parser->error_capacity = 10;
    parser->panic_mode = 0;
    
    parser->errors = malloc(parser->error_capacity * sizeof(parse_error_t));
    if (!parser->errors) {
        free(parser);
        return NULL;
    }
    
    // Initialize with first token
    parser->current_token = lexer_next_token(lexer);
    parser->previous_token.type = TOKEN_EOF;
    parser->previous_token.value = NULL;
    parser->previous_token.line = 0;
    parser->previous_token.column = 0;
    
    return parser;
}

// Destroy parser
void parser_destroy(parser_t* parser) {
    if (!parser) return;
    
    // Free any remaining token values that we allocated
    if (parser->previous_token.value) {
        free(parser->previous_token.value);
    }
    
    for (size_t i = 0; i < parser->error_count; i++) {
        free(parser->errors[i].message);
    }
    free(parser->errors);
    free(parser);
}

// Utility functions
int parser_check(parser_t* parser, token_type_t type) {
    return parser->current_token.type == type;
}

int parser_match(parser_t* parser, token_type_t type) {
    if (parser_check(parser, type)) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

token_t parser_advance(parser_t* parser) {
    // Free the previous token's value if we allocated it
    if (parser->previous_token.value) {
        free(parser->previous_token.value);
        parser->previous_token.value = NULL;
    }
    
    // Copy the current token, including copying its value
    parser->previous_token.type = parser->current_token.type;
    parser->previous_token.line = parser->current_token.line;
    parser->previous_token.column = parser->current_token.column;
    
    if (parser->current_token.value) {
        parser->previous_token.value = malloc(strlen(parser->current_token.value) + 1);
        strcpy(parser->previous_token.value, parser->current_token.value);
    } else {
        parser->previous_token.value = NULL;
    }
    
    if (parser->current_token.type != TOKEN_EOF) {
        parser->current_token = lexer_next_token(parser->lexer);
    }
    
    return parser->previous_token;
}

void parser_consume(parser_t* parser, token_type_t type, const char* error_message) {
    if (parser->current_token.type == type) {
        parser_advance(parser);
        return;
    }
    
    parser_error_at_current(parser, error_message);
}

// Error handling
void parser_error(parser_t* parser, const char* message) {
    parser_error_at_previous(parser, message);
}

void parser_error_at_current(parser_t* parser, const char* message) {
    if (parser->panic_mode) return;
    
    parser->panic_mode = 1;
    
    if (parser->error_count >= parser->error_capacity) {
        parser->error_capacity *= 2;
        parser->errors = realloc(parser->errors, parser->error_capacity * sizeof(parse_error_t));
        if (!parser->errors) {
            fprintf(stderr, "Error: Failed to reallocate error array\n");
            return;
        }
    }
    
    parse_error_t* error = &parser->errors[parser->error_count++];
    error->line = parser->current_token.line;
    error->column = parser->current_token.column;
    error->message = malloc(strlen(message) + 1);
    strcpy(error->message, message);
}

void parser_error_at_previous(parser_t* parser, const char* message) {
    if (parser->panic_mode) return;
    
    parser->panic_mode = 1;
    
    if (parser->error_count >= parser->error_capacity) {
        parser->error_capacity *= 2;
        parser->errors = realloc(parser->errors, parser->error_capacity * sizeof(parse_error_t));
        if (!parser->errors) {
            fprintf(stderr, "Error: Failed to reallocate error array\n");
            return;
        }
    }
    
    parse_error_t* error = &parser->errors[parser->error_count++];
    error->line = parser->previous_token.line;
    error->column = parser->previous_token.column;
    error->message = malloc(strlen(message) + 1);
    strcpy(error->message, message);
}

void parser_synchronize(parser_t* parser) {
    parser->panic_mode = 0;
    
    while (parser->current_token.type != TOKEN_EOF) {
        if (parser->previous_token.type == TOKEN_SEMICOLON) return;
        
        switch (parser->current_token.type) {
            case TOKEN_IF:
            case TOKEN_FOR:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
            case TOKEN_INT:
            case TOKEN_CHAR:
            case TOKEN_VOID:
                return;
            default:
                ; // Do nothing
        }
        
        parser_advance(parser);
    }
}

int parser_has_errors(parser_t* parser) {
    return parser->error_count > 0;
}

void parser_print_errors(parser_t* parser) {
    for (size_t i = 0; i < parser->error_count; i++) {
        fprintf(stderr, "Error at line %d, column %d: %s\n",
                parser->errors[i].line,
                parser->errors[i].column,
                parser->errors[i].message);
    }
}

// Type parsing
data_type_t parser_parse_type(parser_t* parser) {
    if (parser_match(parser, TOKEN_INT)) {
        return TYPE_INT;
    } else if (parser_match(parser, TOKEN_CHAR)) {
        // Check for pointer
        if (parser_match(parser, TOKEN_MULTIPLY)) {
            return TYPE_CHAR_PTR;
        }
        return TYPE_CHAR;
    } else if (parser_match(parser, TOKEN_VOID)) {
        return TYPE_VOID;
    } else {
        parser_error_at_current(parser, "Expected type name");
        return TYPE_VOID;
    }
}

// Convert token type to data type
data_type_t token_to_data_type(int token_type) {
    switch (token_type) {
        case TOKEN_INT: return TYPE_INT;
        case TOKEN_CHAR: return TYPE_CHAR;
        case TOKEN_VOID: return TYPE_VOID;
        default: return TYPE_VOID;
    }
}

// Main parsing function
ast_node_t* parser_parse_program(parser_t* parser) {
    ast_node_t* program = ast_create_program();
    if (!program) return NULL;
    
    while (!parser_check(parser, TOKEN_EOF)) {
        if (parser->error_count >= MAX_PARSE_ERRORS) {
            parser_error_at_current(parser, "Too many parse errors, giving up");
            break;
        }
        
        ast_node_t* declaration = parser_parse_declaration(parser);
        if (declaration) {
            ast_add_declaration(program, declaration);
        }
        
        if (parser->panic_mode) {
            parser_synchronize(parser);
        }
    }
    
    return program;
}

// Parse declaration (function or variable)
ast_node_t* parser_parse_declaration(parser_t* parser) {
    data_type_t type = parser_parse_type(parser);
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error_at_current(parser, "Expected identifier");
        return NULL;
    }
    
    // Make a copy of the identifier name
    char* name = malloc(strlen(parser->current_token.value) + 1);
    strcpy(name, parser->current_token.value);
    parser_advance(parser);
    
    if (parser_check(parser, TOKEN_LEFT_PAREN)) {
        // Function declaration
        ast_node_t* func = parser_parse_function_declaration(parser, type, name);
        free(name);
        return func;
    } else {
        // Variable declaration
        ast_node_t* var = parser_parse_variable_declaration(parser, type, name);
        free(name);
        return var;
    }
}

// Parse function declaration
ast_node_t* parser_parse_function_declaration(parser_t* parser, data_type_t return_type, const char* name) {
    ast_node_t* function = ast_create_function_decl(return_type, name);
    if (!function) return NULL;
    
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name");
    
    // Parse parameters
    if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            ast_node_t* param = parser_parse_parameter(parser);
            if (param) {
                ast_add_parameter(function, param);
            }
        } while (parser_match(parser, TOKEN_COMMA));
    }
    
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters");
    
    // Parse function body (compound statement or semicolon for declaration)
    if (parser_check(parser, TOKEN_SEMICOLON)) {
        parser_advance(parser);
        // Function declaration only, no body
    } else {
        ast_node_t* body = parser_parse_compound_statement(parser);
        function->data.function_decl.body = body;
    }
    
    return function;
}

// Parse variable declaration
ast_node_t* parser_parse_variable_declaration(parser_t* parser, data_type_t var_type, const char* name) {
    ast_node_t* initializer = NULL;
    
    if (parser_match(parser, TOKEN_ASSIGN)) {
        initializer = parser_parse_expression(parser);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration");
    
    return ast_create_variable_decl(var_type, name, initializer);
}

// Parse parameter
ast_node_t* parser_parse_parameter(parser_t* parser) {
    data_type_t param_type = parser_parse_type(parser);
    
    if (!parser_check(parser, TOKEN_IDENTIFIER)) {
        parser_error_at_current(parser, "Expected parameter name");
        return NULL;
    }
    
    // Make a copy of the parameter name
    char* name = malloc(strlen(parser->current_token.value) + 1);
    strcpy(name, parser->current_token.value);
    parser_advance(parser);
    
    ast_node_t* param = ast_create_parameter(param_type, name);
    free(name);
    return param;
}

// Parse statement
ast_node_t* parser_parse_statement(parser_t* parser) {
    if (parser_check(parser, TOKEN_LEFT_BRACE)) {
        return parser_parse_compound_statement(parser);
    }
    if (parser_check(parser, TOKEN_IF)) {
        return parser_parse_if_statement(parser);
    }
    if (parser_check(parser, TOKEN_WHILE)) {
        return parser_parse_while_statement(parser);
    }
    if (parser_check(parser, TOKEN_FOR)) {
        return parser_parse_for_statement(parser);
    }
    if (parser_check(parser, TOKEN_RETURN)) {
        return parser_parse_return_statement(parser);
    }
    if (parser_check(parser, TOKEN_INT) || parser_check(parser, TOKEN_CHAR) || parser_check(parser, TOKEN_VOID)) {
        // Variable declaration inside compound statement
        return parser_parse_declaration(parser);
    }
    
    return parser_parse_expression_statement(parser);
}

// Parse compound statement
ast_node_t* parser_parse_compound_statement(parser_t* parser) {
    parser_consume(parser, TOKEN_LEFT_BRACE, "Expected '{'");
    
    ast_node_t* compound = ast_create_compound_stmt();
    if (!compound) return NULL;
    
    while (!parser_check(parser, TOKEN_RIGHT_BRACE) && !parser_check(parser, TOKEN_EOF)) {
        ast_node_t* stmt = parser_parse_statement(parser);
        if (stmt) {
            ast_add_statement(compound, stmt);
        }
        
        if (parser->panic_mode) {
            parser_synchronize(parser);
        }
    }
    
    parser_consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after block");
    
    return compound;
}

// Parse if statement
ast_node_t* parser_parse_if_statement(parser_t* parser) {
    parser_consume(parser, TOKEN_IF, "Expected 'if'");
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'if'");
    
    ast_node_t* condition = parser_parse_expression(parser);
    
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after if condition");
    
    ast_node_t* then_stmt = parser_parse_statement(parser);
    ast_node_t* else_stmt = NULL;
    
    if (parser_match(parser, TOKEN_ELSE)) {
        else_stmt = parser_parse_statement(parser);
    }
    
    return ast_create_if_stmt(condition, then_stmt, else_stmt);
}

// Parse while statement
ast_node_t* parser_parse_while_statement(parser_t* parser) {
    parser_consume(parser, TOKEN_WHILE, "Expected 'while'");
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'while'");
    
    ast_node_t* condition = parser_parse_expression(parser);
    
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after while condition");
    
    ast_node_t* body = parser_parse_statement(parser);
    
    return ast_create_while_stmt(condition, body);
}

// Parse for statement
ast_node_t* parser_parse_for_statement(parser_t* parser) {
    parser_consume(parser, TOKEN_FOR, "Expected 'for'");
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after 'for'");
    
    // Init statement
    ast_node_t* init = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        if (parser_check(parser, TOKEN_INT) || parser_check(parser, TOKEN_CHAR)) {
            init = parser_parse_declaration(parser);
        } else {
            init = parser_parse_expression_statement(parser);
        }
    } else {
        parser_advance(parser); // consume semicolon
    }
    
    // Condition
    ast_node_t* condition = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        condition = parser_parse_expression(parser);
    }
    parser_consume(parser, TOKEN_SEMICOLON, "Expected ';' after for condition");
    
    // Update
    ast_node_t* update = NULL;
    if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
        update = parser_parse_expression(parser);
    }
    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after for clauses");
    
    ast_node_t* body = parser_parse_statement(parser);
    
    return ast_create_for_stmt(init, condition, update, body);
}

// Parse return statement
ast_node_t* parser_parse_return_statement(parser_t* parser) {
    parser_consume(parser, TOKEN_RETURN, "Expected 'return'");
    
    ast_node_t* value = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        value = parser_parse_expression(parser);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expected ';' after return statement");
    
    return ast_create_return_stmt(value);
}

// Parse expression statement
ast_node_t* parser_parse_expression_statement(parser_t* parser) {
    ast_node_t* expression = NULL;
    
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        expression = parser_parse_expression(parser);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expected ';' after expression");
    
    return ast_create_expression_stmt(expression);
}

// Expression parsing with precedence (recursive descent)
ast_node_t* parser_parse_expression(parser_t* parser) {
    return parser_parse_assignment(parser);
}

ast_node_t* parser_parse_assignment(parser_t* parser) {
    ast_node_t* expr = parser_parse_logical_or(parser);
    
    if (parser_match(parser, TOKEN_ASSIGN)) {
        ast_node_t* value = parser_parse_assignment(parser);
        return ast_create_binary_op("=", expr, value);
    }
    
    return expr;
}

ast_node_t* parser_parse_logical_or(parser_t* parser) {
    ast_node_t* expr = parser_parse_logical_and(parser);
    
    while (parser_match(parser, TOKEN_LOGICAL_OR)) {
        ast_node_t* right = parser_parse_logical_and(parser);
        expr = ast_create_binary_op("||", expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_logical_and(parser_t* parser) {
    ast_node_t* expr = parser_parse_equality(parser);
    
    while (parser_match(parser, TOKEN_LOGICAL_AND)) {
        ast_node_t* right = parser_parse_equality(parser);
        expr = ast_create_binary_op("&&", expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_equality(parser_t* parser) {
    ast_node_t* expr = parser_parse_relational(parser);
    
    while (parser_match(parser, TOKEN_EQUAL) || parser_match(parser, TOKEN_NOT_EQUAL)) {
        char* op = (parser->previous_token.type == TOKEN_EQUAL) ? "==" : "!=";
        ast_node_t* right = parser_parse_relational(parser);
        expr = ast_create_binary_op(op, expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_relational(parser_t* parser) {
    ast_node_t* expr = parser_parse_additive(parser);
    
    while (parser_match(parser, TOKEN_LESS) || parser_match(parser, TOKEN_LESS_EQUAL) ||
           parser_match(parser, TOKEN_GREATER) || parser_match(parser, TOKEN_GREATER_EQUAL)) {
        char* op;
        switch (parser->previous_token.type) {
            case TOKEN_LESS: op = "<"; break;
            case TOKEN_LESS_EQUAL: op = "<="; break;
            case TOKEN_GREATER: op = ">"; break;
            case TOKEN_GREATER_EQUAL: op = ">="; break;
            default: op = "?";
        }
        ast_node_t* right = parser_parse_additive(parser);
        expr = ast_create_binary_op(op, expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_additive(parser_t* parser) {
    ast_node_t* expr = parser_parse_multiplicative(parser);
    
    while (parser_match(parser, TOKEN_PLUS) || parser_match(parser, TOKEN_MINUS)) {
        char* op = (parser->previous_token.type == TOKEN_PLUS) ? "+" : "-";
        ast_node_t* right = parser_parse_multiplicative(parser);
        expr = ast_create_binary_op(op, expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_multiplicative(parser_t* parser) {
    ast_node_t* expr = parser_parse_unary(parser);
    
    while (parser_match(parser, TOKEN_MULTIPLY) || parser_match(parser, TOKEN_DIVIDE) || parser_match(parser, TOKEN_MODULO)) {
        char* op;
        switch (parser->previous_token.type) {
            case TOKEN_MULTIPLY: op = "*"; break;
            case TOKEN_DIVIDE: op = "/"; break;
            case TOKEN_MODULO: op = "%"; break;
            default: op = "?";
        }
        ast_node_t* right = parser_parse_unary(parser);
        expr = ast_create_binary_op(op, expr, right);
    }
    
    return expr;
}

ast_node_t* parser_parse_unary(parser_t* parser) {
    if (parser_match(parser, TOKEN_LOGICAL_NOT) || parser_match(parser, TOKEN_MINUS) || parser_match(parser, TOKEN_PLUS)) {
        char* op;
        switch (parser->previous_token.type) {
            case TOKEN_LOGICAL_NOT: op = "!"; break;
            case TOKEN_MINUS: op = "-"; break;
            case TOKEN_PLUS: op = "+"; break;
            default: op = "?";
        }
        ast_node_t* operand = parser_parse_unary(parser);
        return ast_create_unary_op(op, operand);
    }
    
    return parser_parse_postfix(parser);
}

ast_node_t* parser_parse_postfix(parser_t* parser) {
    ast_node_t* expr = parser_parse_primary(parser);
    
    while (parser_match(parser, TOKEN_LEFT_PAREN)) {
        // Function call
        if (expr->type != AST_IDENTIFIER) {
            parser_error_at_previous(parser, "Can only call identifiers");
            return NULL;
        }
        
        ast_node_t* call = ast_create_function_call(expr->data.identifier.name);
        ast_destroy(expr); // Free the identifier node
        
        if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
            do {
                ast_node_t* arg = parser_parse_expression(parser);
                if (arg) {
                    ast_add_argument(call, arg);
                }
            } while (parser_match(parser, TOKEN_COMMA));
        }
        
        parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after function arguments");
        expr = call;
    }
    
    return expr;
}

ast_node_t* parser_parse_primary(parser_t* parser) {
    if (parser_match(parser, TOKEN_NUMBER)) {
        if (!parser->previous_token.value) {
            return ast_create_number(0);
        }
        int value = atoi(parser->previous_token.value);
        return ast_create_number(value);
    }
    
    if (parser_match(parser, TOKEN_STRING)) {
        // Make a copy of the value
        char* value = malloc(strlen(parser->previous_token.value) + 1);
        strcpy(value, parser->previous_token.value);
        ast_node_t* node = ast_create_string(value);
        free(value);
        return node;
    }
    
    if (parser_match(parser, TOKEN_IDENTIFIER)) {
        // Make a copy of the value
        char* name = malloc(strlen(parser->previous_token.value) + 1);
        strcpy(name, parser->previous_token.value);
        ast_node_t* node = ast_create_identifier(name);
        free(name);
        return node;
    }
    
    if (parser_match(parser, TOKEN_LEFT_PAREN)) {
        ast_node_t* expr = parser_parse_expression(parser);
        parser_consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    parser_error_at_current(parser, "Expected expression");
    return NULL;
}
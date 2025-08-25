// src/parser.h
#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// Maximum number of errors to collect before giving up
#define MAX_PARSE_ERRORS 50

// Parse error structure
typedef struct {
    char* message;
    int line;
    int column;
} parse_error_t;

// Parser state
typedef struct {
    lexer_t* lexer;
    token_t current_token;
    token_t previous_token;
    
    // Error handling
    parse_error_t* errors;
    size_t error_count;
    size_t error_capacity;
    int panic_mode;
} parser_t;

// Parser lifecycle
parser_t* parser_create(lexer_t* lexer);
void parser_destroy(parser_t* parser);

// Main parsing functions
ast_node_t* parser_parse_program(parser_t* parser);

// Grammar rule functions (recursive descent)
ast_node_t* parser_parse_declaration(parser_t* parser);
ast_node_t* parser_parse_function_declaration(parser_t* parser, data_type_t return_type, const char* name);
ast_node_t* parser_parse_variable_declaration(parser_t* parser, data_type_t var_type, const char* name);
ast_node_t* parser_parse_parameter_list(parser_t* parser);
ast_node_t* parser_parse_parameter(parser_t* parser);

ast_node_t* parser_parse_statement(parser_t* parser);
ast_node_t* parser_parse_compound_statement(parser_t* parser);
ast_node_t* parser_parse_if_statement(parser_t* parser);
ast_node_t* parser_parse_while_statement(parser_t* parser);
ast_node_t* parser_parse_for_statement(parser_t* parser);
ast_node_t* parser_parse_return_statement(parser_t* parser);
ast_node_t* parser_parse_expression_statement(parser_t* parser);

ast_node_t* parser_parse_expression(parser_t* parser);
ast_node_t* parser_parse_assignment(parser_t* parser);
ast_node_t* parser_parse_logical_or(parser_t* parser);
ast_node_t* parser_parse_logical_and(parser_t* parser);
ast_node_t* parser_parse_equality(parser_t* parser);
ast_node_t* parser_parse_relational(parser_t* parser);
ast_node_t* parser_parse_additive(parser_t* parser);
ast_node_t* parser_parse_multiplicative(parser_t* parser);
ast_node_t* parser_parse_unary(parser_t* parser);
ast_node_t* parser_parse_postfix(parser_t* parser);
ast_node_t* parser_parse_primary(parser_t* parser);

// Utility functions
data_type_t parser_parse_type(parser_t* parser);
int parser_match(parser_t* parser, token_type_t type);
int parser_check(parser_t* parser, token_type_t type);
token_t parser_advance(parser_t* parser);
void parser_consume(parser_t* parser, token_type_t type, const char* error_message);

// Error handling
void parser_error(parser_t* parser, const char* message);
void parser_error_at_current(parser_t* parser, const char* message);
void parser_error_at_previous(parser_t* parser, const char* message);
void parser_synchronize(parser_t* parser);
int parser_has_errors(parser_t* parser);
void parser_print_errors(parser_t* parser);

#endif // PARSER_H
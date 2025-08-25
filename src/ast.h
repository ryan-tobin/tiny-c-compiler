// src/ast.h
#ifndef AST_H
#define AST_H

#include <stddef.h>

// AST node types
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION_DECL,
    AST_VARIABLE_DECL,
    AST_PARAMETER,
    AST_COMPOUND_STMT,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_RETURN_STMT,
    AST_EXPRESSION_STMT,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_FUNCTION_CALL,
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING
} ast_node_type_t;

// Data types in our language
typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_VOID,
    TYPE_CHAR_PTR
} data_type_t;

// Forward declaration
typedef struct ast_node ast_node_t;

// AST node structure
struct ast_node {
    ast_node_type_t type;
    data_type_t data_type;
    
    union {
        struct {
            ast_node_t** declarations;
            size_t declaration_count;
        } program;
        
        struct {
            data_type_t return_type;
            char* name;
            ast_node_t** parameters;
            size_t parameter_count;
            ast_node_t* body;
        } function_decl;
        
        struct {
            data_type_t var_type;
            char* name;
            ast_node_t* initializer;
        } variable_decl;
        
        struct {
            data_type_t param_type;
            char* name;
        } parameter;
        
        struct {
            ast_node_t** statements;
            size_t statement_count;
        } compound_stmt;
        
        struct {
            ast_node_t* condition;
            ast_node_t* then_stmt;
            ast_node_t* else_stmt;
        } if_stmt;
        
        struct {
            ast_node_t* condition;
            ast_node_t* body;
        } while_stmt;
        
        struct {
            ast_node_t* init;
            ast_node_t* condition;
            ast_node_t* update;
            ast_node_t* body;
        } for_stmt;
        
        struct {
            ast_node_t* value;
        } return_stmt;
        
        struct {
            ast_node_t* expression;
        } expression_stmt;
        
        struct {
            char* oper;
            ast_node_t* left;
            ast_node_t* right;
        } binary_op;
        
        struct {
            char* oper;
            ast_node_t* operand;
        } unary_op;
        
        struct {
            char* name;
            ast_node_t** arguments;
            size_t argument_count;
        } function_call;
        
        struct {
            char* name;
        } identifier;
        
        struct {
            int value;
        } number;
        
        struct {
            char* value;
        } string;
    } data;
};

// Core AST functions
ast_node_t* ast_create_node(ast_node_type_t type);
void ast_destroy(ast_node_t* node);
void ast_print(ast_node_t* node, int indent);
const char* ast_node_type_to_string(ast_node_type_t type);
const char* data_type_to_string(data_type_t type);

// Convenience constructors for specific node types
ast_node_t* ast_create_program(void);
ast_node_t* ast_create_function_decl(data_type_t return_type, const char* name);
ast_node_t* ast_create_variable_decl(data_type_t var_type, const char* name, ast_node_t* initializer);
ast_node_t* ast_create_parameter(data_type_t param_type, const char* name);
ast_node_t* ast_create_compound_stmt(void);
ast_node_t* ast_create_if_stmt(ast_node_t* condition, ast_node_t* then_stmt, ast_node_t* else_stmt);
ast_node_t* ast_create_while_stmt(ast_node_t* condition, ast_node_t* body);
ast_node_t* ast_create_for_stmt(ast_node_t* init, ast_node_t* condition, ast_node_t* update, ast_node_t* body);
ast_node_t* ast_create_return_stmt(ast_node_t* value);
ast_node_t* ast_create_expression_stmt(ast_node_t* expression);
ast_node_t* ast_create_binary_op(const char* oper, ast_node_t* left, ast_node_t* right);
ast_node_t* ast_create_unary_op(const char* oper, ast_node_t* operand);
ast_node_t* ast_create_function_call(const char* name);
ast_node_t* ast_create_identifier(const char* name);
ast_node_t* ast_create_number(int value);
ast_node_t* ast_create_string(const char* value);

// Helper functions for dynamic arrays
void ast_add_declaration(ast_node_t* program, ast_node_t* declaration);
void ast_add_parameter(ast_node_t* function, ast_node_t* parameter);
void ast_add_statement(ast_node_t* compound, ast_node_t* statement);
void ast_add_argument(ast_node_t* function_call, ast_node_t* argument);

// Type conversion helpers
data_type_t token_to_data_type(int token_type);

#endif // AST_H
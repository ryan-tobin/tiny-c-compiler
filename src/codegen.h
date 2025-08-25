// src/codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "semantic.h"
#include <stdio.h>

// Maximum number of registers we can use
#define MAX_REGISTERS 8

// Register allocation
typedef enum {
    REG_NONE = -1,
    REG_RAX = 0,  // Return value, scratch
    REG_RBX = 1,  // Callee-saved
    REG_RCX = 2,  // 4th argument, scratch  
    REG_RDX = 3,  // 3rd argument, scratch
    REG_RSI = 4,  // 2nd argument
    REG_RDI = 5,  // 1st argument
    REG_R8  = 6,  // 5th argument, scratch
    REG_R9  = 7   // 6th argument, scratch
} register_t;

// Stack frame management
typedef struct {
    int offset;           // Offset from base pointer
    data_type_t type;     // Variable type
    char* name;           // Variable name
} stack_var_t;

// Function context
typedef struct {
    char* name;
    int stack_size;       // Total stack space needed
    stack_var_t* variables;
    size_t variable_count;
    size_t variable_capacity;
    int label_counter;    // For generating unique labels
} function_context_t;

// String literal entry
typedef struct {
    char* value;
    char* label;
} string_literal_t;

// Code generator state
typedef struct {
    FILE* output;
    function_context_t* current_function;
    int string_counter;   // For generating string labels
    int label_counter;    // Global label counter
    
    // Register allocation (simple)
    int register_used[MAX_REGISTERS];
    int next_temp_register;
    
    // String literals table
    string_literal_t* string_literals;
    size_t string_literal_count;
    size_t string_literal_capacity;
} codegen_t;

// Code generator lifecycle
codegen_t* codegen_create(const char* output_filename);
void codegen_destroy(codegen_t* codegen);

// Main code generation
int codegen_generate(codegen_t* codegen, ast_node_t* ast);

// AST code generation functions
void codegen_program(codegen_t* codegen, ast_node_t* node);
void codegen_function_decl(codegen_t* codegen, ast_node_t* node);
void codegen_statement(codegen_t* codegen, ast_node_t* node);
void codegen_compound_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_if_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_while_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_for_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_return_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_expression_stmt(codegen_t* codegen, ast_node_t* node);
void codegen_variable_decl(codegen_t* codegen, ast_node_t* node);

// Expression code generation (returns register holding result)
register_t codegen_expression(codegen_t* codegen, ast_node_t* node);
register_t codegen_binary_op(codegen_t* codegen, ast_node_t* node);
register_t codegen_unary_op(codegen_t* codegen, ast_node_t* node);
register_t codegen_function_call(codegen_t* codegen, ast_node_t* node);
register_t codegen_identifier(codegen_t* codegen, ast_node_t* node);
register_t codegen_number(codegen_t* codegen, ast_node_t* node);
register_t codegen_string(codegen_t* codegen, ast_node_t* node);

// Assembly output helpers
void codegen_emit(codegen_t* codegen, const char* format, ...);
void codegen_emit_label(codegen_t* codegen, const char* label);
void codegen_emit_comment(codegen_t* codegen, const char* comment);

// Register management
register_t codegen_allocate_register(codegen_t* codegen);
void codegen_free_register(codegen_t* codegen, register_t reg);
void codegen_free_all_registers(codegen_t* codegen);
const char* codegen_register_name(register_t reg, int size);

// Function context management
function_context_t* function_context_create(const char* name);
void function_context_destroy(function_context_t* context);
int function_context_add_variable(function_context_t* context, const char* name, data_type_t type);
stack_var_t* function_context_find_variable(function_context_t* context, const char* name);

// Label generation
char* codegen_generate_label(codegen_t* codegen, const char* prefix);
char* codegen_generate_string_label(codegen_t* codegen);

// String literal management
const char* codegen_add_string_literal(codegen_t* codegen, const char* value);

// Utility functions
int codegen_type_size(data_type_t type);
const char* codegen_type_suffix(data_type_t type);

#endif // CODEGEN_H
// src/semantic.h
#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

// Maximum number of semantic errors to collect
#define MAX_SEMANTIC_ERRORS 100

// Maximum nesting depth for scopes
#define MAX_SCOPE_DEPTH 32

// Symbol types
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} symbol_type_t;

// Symbol table entry
typedef struct symbol {
    char* name;
    symbol_type_t type;
    data_type_t data_type;
    int scope_level;
    
    // Additional info for functions
    struct {
        data_type_t* parameter_types;
        size_t parameter_count;
        int defined;  // 1 if function has body, 0 if just declared
    } function_info;
    
    struct symbol* next; // For hash table chaining
} symbol_t;

// Symbol table (hash table)
#define SYMBOL_TABLE_SIZE 256

typedef struct {
    symbol_t* buckets[SYMBOL_TABLE_SIZE];
} symbol_table_t;

// Scope stack
typedef struct scope {
    symbol_table_t* symbols;
    int level;
    struct scope* parent;
} scope_t;

// Semantic error
typedef struct {
    char* message;
    int line;
    int column;
    char* context;
} semantic_error_t;

// Semantic analyzer state
typedef struct {
    scope_t* current_scope;
    int scope_level;
    
    // Error handling
    semantic_error_t* errors;
    size_t error_count;
    size_t error_capacity;
    
    // Current function context (for return type checking)
    data_type_t current_function_return_type;
    char* current_function_name;
} semantic_analyzer_t;

// Semantic analyzer lifecycle
semantic_analyzer_t* semantic_create(void);
void semantic_destroy(semantic_analyzer_t* analyzer);

// Main semantic analysis
int semantic_analyze(semantic_analyzer_t* analyzer, ast_node_t* ast);

// Symbol table operations
symbol_table_t* symbol_table_create(void);
void symbol_table_destroy(symbol_table_t* table);
symbol_t* symbol_table_lookup(symbol_table_t* table, const char* name);
int symbol_table_insert(symbol_table_t* table, symbol_t* symbol);
void symbol_table_remove(symbol_table_t* table, const char* name);

// Symbol operations
symbol_t* symbol_create(const char* name, symbol_type_t type, data_type_t data_type);
void symbol_destroy(symbol_t* symbol);

// Scope management
scope_t* scope_create(scope_t* parent, int level);
void scope_destroy(scope_t* scope);
void semantic_push_scope(semantic_analyzer_t* analyzer);
void semantic_pop_scope(semantic_analyzer_t* analyzer);
symbol_t* semantic_lookup_symbol(semantic_analyzer_t* analyzer, const char* name);
int semantic_declare_symbol(semantic_analyzer_t* analyzer, symbol_t* symbol);

// AST analysis functions
int semantic_analyze_program(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_function_decl(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_variable_decl(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_statement(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_compound_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_if_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_while_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_for_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_return_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
int semantic_analyze_expression_stmt(semantic_analyzer_t* analyzer, ast_node_t* node);
data_type_t semantic_analyze_expression(semantic_analyzer_t* analyzer, ast_node_t* node);
data_type_t semantic_analyze_binary_op(semantic_analyzer_t* analyzer, ast_node_t* node);
data_type_t semantic_analyze_unary_op(semantic_analyzer_t* analyzer, ast_node_t* node);
data_type_t semantic_analyze_function_call(semantic_analyzer_t* analyzer, ast_node_t* node);
data_type_t semantic_analyze_identifier(semantic_analyzer_t* analyzer, ast_node_t* node);

// Type checking utilities
int semantic_types_compatible(data_type_t type1, data_type_t type2);
int semantic_type_is_numeric(data_type_t type);
int semantic_type_is_boolean_context(data_type_t type);
data_type_t semantic_get_binary_result_type(const char* operator, data_type_t left, data_type_t right);
data_type_t semantic_get_unary_result_type(const char* operator, data_type_t operand);

// Error handling
void semantic_error(semantic_analyzer_t* analyzer, const char* message, ast_node_t* node);
void semantic_error_at(semantic_analyzer_t* analyzer, const char* message, int line, int column, const char* context);
int semantic_has_errors(semantic_analyzer_t* analyzer);
void semantic_print_errors(semantic_analyzer_t* analyzer);

// Utility functions
const char* symbol_type_to_string(symbol_type_t type);
unsigned int hash_string(const char* str);

#endif // SEMANTIC_H
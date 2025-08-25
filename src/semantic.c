// src/semantic.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

// Hash function for symbol table
unsigned int hash_string(const char* str) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    
    return hash % SYMBOL_TABLE_SIZE;
}

// Symbol table operations
symbol_table_t* symbol_table_create(void) {
    symbol_table_t* table = malloc(sizeof(symbol_table_t));
    if (!table) return NULL;
    
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
    
    return table;
}

void symbol_table_destroy(symbol_table_t* table) {
    if (!table) return;
    
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        symbol_t* current = table->buckets[i];
        while (current) {
            symbol_t* next = current->next;
            symbol_destroy(current);
            current = next;
        }
    }
    
    free(table);
}

symbol_t* symbol_table_lookup(symbol_table_t* table, const char* name) {
    if (!table || !name) return NULL;
    
    unsigned int index = hash_string(name);
    symbol_t* current = table->buckets[index];
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int symbol_table_insert(symbol_table_t* table, symbol_t* symbol) {
    if (!table || !symbol) return 0;
    
    // Check if symbol already exists
    if (symbol_table_lookup(table, symbol->name)) {
        return 0; // Symbol already exists
    }
    
    unsigned int index = hash_string(symbol->name);
    symbol->next = table->buckets[index];
    table->buckets[index] = symbol;
    
    return 1;
}

void symbol_table_remove(symbol_table_t* table, const char* name) {
    if (!table || !name) return;
    
    unsigned int index = hash_string(name);
    symbol_t* current = table->buckets[index];
    symbol_t* prev = NULL;
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->buckets[index] = current->next;
            }
            symbol_destroy(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Symbol operations
symbol_t* symbol_create(const char* name, symbol_type_t type, data_type_t data_type) {
    symbol_t* symbol = malloc(sizeof(symbol_t));
    if (!symbol) return NULL;
    
    symbol->name = malloc(strlen(name) + 1);
    if (!symbol->name) {
        free(symbol);
        return NULL;
    }
    strcpy(symbol->name, name);
    
    symbol->type = type;
    symbol->data_type = data_type;
    symbol->scope_level = 0;
    symbol->next = NULL;
    
    // Initialize function info
    symbol->function_info.parameter_types = NULL;
    symbol->function_info.parameter_count = 0;
    symbol->function_info.defined = 0;
    
    return symbol;
}

void symbol_destroy(symbol_t* symbol) {
    if (!symbol) return;
    
    free(symbol->name);
    if (symbol->function_info.parameter_types) {
        free(symbol->function_info.parameter_types);
    }
    free(symbol);
}

// Scope operations
scope_t* scope_create(scope_t* parent, int level) {
    scope_t* scope = malloc(sizeof(scope_t));
    if (!scope) return NULL;
    
    scope->symbols = symbol_table_create();
    if (!scope->symbols) {
        free(scope);
        return NULL;
    }
    
    scope->level = level;
    scope->parent = parent;
    
    return scope;
}

void scope_destroy(scope_t* scope) {
    if (!scope) return;
    
    symbol_table_destroy(scope->symbols);
    free(scope);
}

// Semantic analyzer lifecycle
semantic_analyzer_t* semantic_create(void) {
    semantic_analyzer_t* analyzer = malloc(sizeof(semantic_analyzer_t));
    if (!analyzer) return NULL;
    
    analyzer->current_scope = NULL;
    analyzer->scope_level = 0;
    analyzer->error_count = 0;
    analyzer->error_capacity = 10;
    analyzer->current_function_return_type = TYPE_VOID;
    analyzer->current_function_name = NULL;
    
    analyzer->errors = malloc(analyzer->error_capacity * sizeof(semantic_error_t));
    if (!analyzer->errors) {
        free(analyzer);
        return NULL;
    }
    
    // Create global scope
    semantic_push_scope(analyzer);
    
    return analyzer;
}

void semantic_destroy(semantic_analyzer_t* analyzer) {
    if (!analyzer) return;
    
    // Clean up all scopes
    while (analyzer->current_scope) {
        semantic_pop_scope(analyzer);
    }
    
    // Clean up errors
    for (size_t i = 0; i < analyzer->error_count; i++) {
        free(analyzer->errors[i].message);
        free(analyzer->errors[i].context);
    }
    free(analyzer->errors);
    
    if (analyzer->current_function_name) {
        free(analyzer->current_function_name);
    }
    
    free(analyzer);
}

// Scope management
void semantic_push_scope(semantic_analyzer_t* analyzer) {
    if (!analyzer) return;
    
    scope_t* new_scope = scope_create(analyzer->current_scope, analyzer->scope_level + 1);
    if (!new_scope) {
        fprintf(stderr, "Error: Failed to create new scope\n");
        return;
    }
    
    analyzer->current_scope = new_scope;
    analyzer->scope_level++;
}

void semantic_pop_scope(semantic_analyzer_t* analyzer) {
    if (!analyzer || !analyzer->current_scope) return;
    
    scope_t* old_scope = analyzer->current_scope;
    analyzer->current_scope = old_scope->parent;
    analyzer->scope_level--;
    
    scope_destroy(old_scope);
}

symbol_t* semantic_lookup_symbol(semantic_analyzer_t* analyzer, const char* name) {
    if (!analyzer || !name) return NULL;
    
    scope_t* current = analyzer->current_scope;
    while (current) {
        symbol_t* symbol = symbol_table_lookup(current->symbols, name);
        if (symbol) {
            return symbol;
        }
        current = current->parent;
    }
    
    return NULL;
}

int semantic_declare_symbol(semantic_analyzer_t* analyzer, symbol_t* symbol) {
    if (!analyzer || !symbol || !analyzer->current_scope) return 0;
    
    // Check if symbol already exists in current scope
    symbol_t* existing = symbol_table_lookup(analyzer->current_scope->symbols, symbol->name);
    if (existing) {
        return 0; // Symbol already declared in this scope
    }
    
    symbol->scope_level = analyzer->scope_level;
    return symbol_table_insert(analyzer->current_scope->symbols, symbol);
}

// Error handling
void semantic_error(semantic_analyzer_t* analyzer, const char* message, ast_node_t* node) {
    if (!analyzer || !message) return;
    
    // For now, we don't have line/column info in AST nodes, so use 0
    semantic_error_at(analyzer, message, 0, 0, node ? "AST node" : "unknown");
}

void semantic_error_at(semantic_analyzer_t* analyzer, const char* message, int line, int column, const char* context) {
    if (!analyzer || !message) return;
    
    if (analyzer->error_count >= analyzer->error_capacity) {
        analyzer->error_capacity *= 2;
        analyzer->errors = realloc(analyzer->errors, 
                                 analyzer->error_capacity * sizeof(semantic_error_t));
        if (!analyzer->errors) {
            fprintf(stderr, "Error: Failed to reallocate error array\n");
            return;
        }
    }
    
    semantic_error_t* error = &analyzer->errors[analyzer->error_count++];
    error->line = line;
    error->column = column;
    
    error->message = malloc(strlen(message) + 1);
    strcpy(error->message, message);
    
    if (context) {
        error->context = malloc(strlen(context) + 1);
        strcpy(error->context, context);
    } else {
        error->context = NULL;
    }
}

int semantic_has_errors(semantic_analyzer_t* analyzer) {
    return analyzer && analyzer->error_count > 0;
}

void semantic_print_errors(semantic_analyzer_t* analyzer) {
    if (!analyzer) return;
    
    for (size_t i = 0; i < analyzer->error_count; i++) {
        fprintf(stderr, "Semantic error");
        if (analyzer->errors[i].line > 0) {
            fprintf(stderr, " at line %d, column %d", 
                    analyzer->errors[i].line, analyzer->errors[i].column);
        }
        if (analyzer->errors[i].context) {
            fprintf(stderr, " in %s", analyzer->errors[i].context);
        }
        fprintf(stderr, ": %s\n", analyzer->errors[i].message);
    }
}

// Main semantic analysis
int semantic_analyze(semantic_analyzer_t* analyzer, ast_node_t* ast) {
    if (!analyzer || !ast) return 0;
    
    return semantic_analyze_program(analyzer, ast);
}

// AST analysis functions
int semantic_analyze_program(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_PROGRAM) return 0;
    
    int success = 1;
    
    // First pass: declare all functions (for forward references)
    for (size_t i = 0; i < node->data.program.declaration_count; i++) {
        ast_node_t* decl = node->data.program.declarations[i];
        if (decl->type == AST_FUNCTION_DECL) {
            // Create function symbol
            symbol_t* symbol = symbol_create(
                decl->data.function_decl.name,
                SYMBOL_FUNCTION,
                decl->data.function_decl.return_type
            );
            
            if (!symbol) {
                success = 0;
                continue;
            }
            
            // Set up parameter types
            symbol->function_info.parameter_count = decl->data.function_decl.parameter_count;
            if (symbol->function_info.parameter_count > 0) {
                symbol->function_info.parameter_types = malloc(
                    symbol->function_info.parameter_count * sizeof(data_type_t)
                );
                
                for (size_t j = 0; j < symbol->function_info.parameter_count; j++) {
                    ast_node_t* param = decl->data.function_decl.parameters[j];
                    symbol->function_info.parameter_types[j] = param->data.parameter.param_type;
                }
            }
            
            symbol->function_info.defined = (decl->data.function_decl.body != NULL);
            
            if (!semantic_declare_symbol(analyzer, symbol)) {
                char error_msg[256];
                snprintf(error_msg, sizeof(error_msg), 
                        "Function '%s' already declared", decl->data.function_decl.name);
                semantic_error(analyzer, error_msg, decl);
                symbol_destroy(symbol);
                success = 0;
            }
        }
    }
    
    // Second pass: analyze function bodies and global variables
    for (size_t i = 0; i < node->data.program.declaration_count; i++) {
        ast_node_t* decl = node->data.program.declarations[i];
        
        if (decl->type == AST_FUNCTION_DECL) {
            if (!semantic_analyze_function_decl(analyzer, decl)) {
                success = 0;
            }
        } else if (decl->type == AST_VARIABLE_DECL) {
            if (!semantic_analyze_variable_decl(analyzer, decl)) {
                success = 0;
            }
        }
    }
    
    return success;
}

int semantic_analyze_function_decl(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_DECL) return 0;
    
    // Set current function context
    analyzer->current_function_return_type = node->data.function_decl.return_type;
    if (analyzer->current_function_name) {
        free(analyzer->current_function_name);
    }
    analyzer->current_function_name = malloc(strlen(node->data.function_decl.name) + 1);
    strcpy(analyzer->current_function_name, node->data.function_decl.name);
    
    // If function has no body, it's just a declaration
    if (!node->data.function_decl.body) {
        return 1;
    }
    
    // Create new scope for function
    semantic_push_scope(analyzer);
    
    int success = 1;
    
    // Add parameters to function scope
    for (size_t i = 0; i < node->data.function_decl.parameter_count; i++) {
        ast_node_t* param = node->data.function_decl.parameters[i];
        
        symbol_t* symbol = symbol_create(
            param->data.parameter.name,
            SYMBOL_PARAMETER,
            param->data.parameter.param_type
        );
        
        if (!symbol || !semantic_declare_symbol(analyzer, symbol)) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Parameter '%s' already declared", param->data.parameter.name);
            semantic_error(analyzer, error_msg, param);
            if (symbol) symbol_destroy(symbol);
            success = 0;
        }
    }
    
    // Analyze function body
    if (!semantic_analyze_compound_stmt(analyzer, node->data.function_decl.body)) {
        success = 0;
    }
    
    // Pop function scope
    semantic_pop_scope(analyzer);
    
    // Clear function context
    analyzer->current_function_return_type = TYPE_VOID;
    if (analyzer->current_function_name) {
        free(analyzer->current_function_name);
        analyzer->current_function_name = NULL;
    }
    
    return success;
}

int semantic_analyze_variable_decl(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_VARIABLE_DECL) return 0;
    
    // Create variable symbol
    symbol_t* symbol = symbol_create(
        node->data.variable_decl.name,
        SYMBOL_VARIABLE,
        node->data.variable_decl.var_type
    );
    
    if (!symbol) return 0;
    
    if (!semantic_declare_symbol(analyzer, symbol)) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Variable '%s' already declared", node->data.variable_decl.name);
        semantic_error(analyzer, error_msg, node);
        symbol_destroy(symbol);
        return 0;
    }
    
    // Check initializer if present
    if (node->data.variable_decl.initializer) {
        data_type_t init_type = semantic_analyze_expression(analyzer, node->data.variable_decl.initializer);
        
        if (!semantic_types_compatible(node->data.variable_decl.var_type, init_type)) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Cannot initialize variable '%s' of type '%s' with expression of type '%s'",
                    node->data.variable_decl.name,
                    data_type_to_string(node->data.variable_decl.var_type),
                    data_type_to_string(init_type));
            semantic_error(analyzer, error_msg, node->data.variable_decl.initializer);
            return 0;
        }
    }
    
    return 1;
}

int semantic_analyze_statement(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node) return 1;
    
    switch (node->type) {
        case AST_COMPOUND_STMT:
            return semantic_analyze_compound_stmt(analyzer, node);
        case AST_IF_STMT:
            return semantic_analyze_if_stmt(analyzer, node);
        case AST_WHILE_STMT:
            return semantic_analyze_while_stmt(analyzer, node);
        case AST_FOR_STMT:
            return semantic_analyze_for_stmt(analyzer, node);
        case AST_RETURN_STMT:
            return semantic_analyze_return_stmt(analyzer, node);
        case AST_EXPRESSION_STMT:
            return semantic_analyze_expression_stmt(analyzer, node);
        case AST_VARIABLE_DECL:
            return semantic_analyze_variable_decl(analyzer, node);
        default:
            semantic_error(analyzer, "Unknown statement type", node);
            return 0;
    }
}

int semantic_analyze_compound_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_COMPOUND_STMT) return 0;
    
    // Create new scope for compound statement
    semantic_push_scope(analyzer);
    
    int success = 1;
    
    for (size_t i = 0; i < node->data.compound_stmt.statement_count; i++) {
        if (!semantic_analyze_statement(analyzer, node->data.compound_stmt.statements[i])) {
            success = 0;
        }
    }
    
    // Pop compound statement scope
    semantic_pop_scope(analyzer);
    
    return success;
}

int semantic_analyze_if_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_IF_STMT) return 0;
    
    int success = 1;
    
    // Analyze condition
    data_type_t cond_type = semantic_analyze_expression(analyzer, node->data.if_stmt.condition);
    if (!semantic_type_is_boolean_context(cond_type)) {
        semantic_error(analyzer, "If condition must be boolean expression", node->data.if_stmt.condition);
        success = 0;
    }
    
    // Analyze then branch
    if (!semantic_analyze_statement(analyzer, node->data.if_stmt.then_stmt)) {
        success = 0;
    }
    
    // Analyze else branch if present
    if (node->data.if_stmt.else_stmt) {
        if (!semantic_analyze_statement(analyzer, node->data.if_stmt.else_stmt)) {
            success = 0;
        }
    }
    
    return success;
}

int semantic_analyze_while_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_WHILE_STMT) return 0;
    
    int success = 1;
    
    // Analyze condition
    data_type_t cond_type = semantic_analyze_expression(analyzer, node->data.while_stmt.condition);
    if (!semantic_type_is_boolean_context(cond_type)) {
        semantic_error(analyzer, "While condition must be boolean expression", node->data.while_stmt.condition);
        success = 0;
    }
    
    // Analyze body
    if (!semantic_analyze_statement(analyzer, node->data.while_stmt.body)) {
        success = 0;
    }
    
    return success;
}

int semantic_analyze_for_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_FOR_STMT) return 0;
    
    // Create new scope for for loop
    semantic_push_scope(analyzer);
    
    int success = 1;
    
    // Analyze init
    if (node->data.for_stmt.init) {
        if (!semantic_analyze_statement(analyzer, node->data.for_stmt.init)) {
            success = 0;
        }
    }
    
    // Analyze condition
    if (node->data.for_stmt.condition) {
        data_type_t cond_type = semantic_analyze_expression(analyzer, node->data.for_stmt.condition);
        if (!semantic_type_is_boolean_context(cond_type)) {
            semantic_error(analyzer, "For condition must be boolean expression", node->data.for_stmt.condition);
            success = 0;
        }
    }
    
    // Analyze update
    if (node->data.for_stmt.update) {
        semantic_analyze_expression(analyzer, node->data.for_stmt.update);
    }
    
    // Analyze body
    if (!semantic_analyze_statement(analyzer, node->data.for_stmt.body)) {
        success = 0;
    }
    
    semantic_pop_scope(analyzer);
    
    return success;
}

int semantic_analyze_return_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_RETURN_STMT) return 0;
    
    if (node->data.return_stmt.value) {
        data_type_t return_type = semantic_analyze_expression(analyzer, node->data.return_stmt.value);
        
        if (!semantic_types_compatible(analyzer->current_function_return_type, return_type)) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Return type '%s' does not match function return type '%s'",
                    data_type_to_string(return_type),
                    data_type_to_string(analyzer->current_function_return_type));
            semantic_error(analyzer, error_msg, node->data.return_stmt.value);
            return 0;
        }
    } else {
        // Empty return
        if (analyzer->current_function_return_type != TYPE_VOID) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Function '%s' must return a value",
                    analyzer->current_function_name ? analyzer->current_function_name : "unknown");
            semantic_error(analyzer, error_msg, node);
            return 0;
        }
    }
    
    return 1;
}

int semantic_analyze_expression_stmt(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_EXPRESSION_STMT) return 0;
    
    if (node->data.expression_stmt.expression) {
        semantic_analyze_expression(analyzer, node->data.expression_stmt.expression);
    }
    
    return 1;
}

data_type_t semantic_analyze_expression(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node) return TYPE_VOID;
    
    switch (node->type) {
        case AST_BINARY_OP:
            return semantic_analyze_binary_op(analyzer, node);
        case AST_UNARY_OP:
            return semantic_analyze_unary_op(analyzer, node);
        case AST_FUNCTION_CALL:
            return semantic_analyze_function_call(analyzer, node);
        case AST_IDENTIFIER:
            return semantic_analyze_identifier(analyzer, node);
        case AST_NUMBER:
            return TYPE_INT;
        case AST_STRING:
            return TYPE_CHAR_PTR;
        default:
            semantic_error(analyzer, "Unknown expression type", node);
            return TYPE_VOID;
    }
}

data_type_t semantic_analyze_binary_op(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_BINARY_OP) return TYPE_VOID;
    
    data_type_t left_type = semantic_analyze_expression(analyzer, node->data.binary_op.left);
    data_type_t right_type = semantic_analyze_expression(analyzer, node->data.binary_op.right);
    
    data_type_t result_type = semantic_get_binary_result_type(
        node->data.binary_op.oper, left_type, right_type);
    
    if (result_type == TYPE_VOID) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Cannot apply oper '%s' to types '%s' and '%s'",
                node->data.binary_op.oper,
                data_type_to_string(left_type),
                data_type_to_string(right_type));
        semantic_error(analyzer, error_msg, node);
        return TYPE_VOID;
    }
    
    return result_type;
}

data_type_t semantic_analyze_unary_op(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_UNARY_OP) return TYPE_VOID;
    
    data_type_t operand_type = semantic_analyze_expression(analyzer, node->data.unary_op.operand);
    data_type_t result_type = semantic_get_unary_result_type(node->data.unary_op.oper, operand_type);
    
    if (result_type == TYPE_VOID) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Cannot apply unary oper '%s' to type '%s'",
                node->data.unary_op.oper,
                data_type_to_string(operand_type));
        semantic_error(analyzer, error_msg, node);
        return TYPE_VOID;
    }
    
    return result_type;
}

data_type_t semantic_analyze_function_call(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_FUNCTION_CALL) return TYPE_VOID;
    
    // Look up function
    symbol_t* func_symbol = semantic_lookup_symbol(analyzer, node->data.function_call.name);
    if (!func_symbol) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Undefined function '%s'", node->data.function_call.name);
        semantic_error(analyzer, error_msg, node);
        return TYPE_VOID;
    }
    
    if (func_symbol->type != SYMBOL_FUNCTION) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "'%s' is not a function", node->data.function_call.name);
        semantic_error(analyzer, error_msg, node);
        return TYPE_VOID;
    }
    
    // Check argument count
    if (node->data.function_call.argument_count != func_symbol->function_info.parameter_count) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Function '%s' expects %zu arguments, got %zu",
                node->data.function_call.name,
                func_symbol->function_info.parameter_count,
                node->data.function_call.argument_count);
        semantic_error(analyzer, error_msg, node);
        return func_symbol->data_type;
    }
    
    // Check argument types
    for (size_t i = 0; i < node->data.function_call.argument_count; i++) {
        data_type_t arg_type = semantic_analyze_expression(analyzer, node->data.function_call.arguments[i]);
        data_type_t param_type = func_symbol->function_info.parameter_types[i];
        
        if (!semantic_types_compatible(param_type, arg_type)) {
            char error_msg[256];
            snprintf(error_msg, sizeof(error_msg), 
                    "Argument %zu to function '%s' has type '%s', expected '%s'",
                    i + 1, node->data.function_call.name,
                    data_type_to_string(arg_type),
                    data_type_to_string(param_type));
            semantic_error(analyzer, error_msg, node->data.function_call.arguments[i]);
        }
    }
    
    return func_symbol->data_type;
}

data_type_t semantic_analyze_identifier(semantic_analyzer_t* analyzer, ast_node_t* node) {
    if (!node || node->type != AST_IDENTIFIER) return TYPE_VOID;
    
    symbol_t* symbol = semantic_lookup_symbol(analyzer, node->data.identifier.name);
    if (!symbol) {
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), 
                "Undefined identifier '%s'", node->data.identifier.name);
        semantic_error(analyzer, error_msg, node);
        return TYPE_VOID;
    }
    
    return symbol->data_type;
}

// Type checking utilities
int semantic_types_compatible(data_type_t type1, data_type_t type2) {
    return type1 == type2;
}

int semantic_type_is_numeric(data_type_t type) {
    return type == TYPE_INT || type == TYPE_CHAR;
}

int semantic_type_is_boolean_context(data_type_t type) {
    return semantic_type_is_numeric(type);
}

data_type_t semantic_get_binary_result_type(const char* oper, data_type_t left, data_type_t right) {
    // Assignment
    if (strcmp(oper, "=") == 0) {
        return semantic_types_compatible(left, right) ? left : TYPE_VOID;
    }
    
    // Arithmetic operations
    if (strcmp(oper, "+") == 0 || strcmp(oper, "-") == 0 || 
        strcmp(oper, "*") == 0 || strcmp(oper, "/") == 0 || strcmp(oper, "%") == 0) {
        if (semantic_type_is_numeric(left) && semantic_type_is_numeric(right)) {
            return TYPE_INT;
        }
        return TYPE_VOID;
    }
    
    // Comparison operations
    if (strcmp(oper, "<") == 0 || strcmp(oper, ">") == 0 || 
        strcmp(oper, "<=") == 0 || strcmp(oper, ">=") == 0 ||
        strcmp(oper, "==") == 0 || strcmp(oper, "!=") == 0) {
        if (semantic_types_compatible(left, right)) {
            return TYPE_INT; // Boolean result represented as int
        }
        return TYPE_VOID;
    }
    
    // Logical operations
    if (strcmp(oper, "&&") == 0 || strcmp(oper, "||") == 0) {
        if (semantic_type_is_boolean_context(left) && semantic_type_is_boolean_context(right)) {
            return TYPE_INT;
        }
        return TYPE_VOID;
    }
    
    return TYPE_VOID;
}

data_type_t semantic_get_unary_result_type(const char* oper, data_type_t operand) {
    if (strcmp(oper, "-") == 0 || strcmp(oper, "+") == 0) {
        return semantic_type_is_numeric(operand) ? TYPE_INT : TYPE_VOID;
    }
    
    if (strcmp(oper, "!") == 0) {
        return semantic_type_is_boolean_context(operand) ? TYPE_INT : TYPE_VOID;
    }
    
    return TYPE_VOID;
}

// Utility functions
const char* symbol_type_to_string(symbol_type_t type) {
    switch (type) {
        case SYMBOL_VARIABLE: return "variable";
        case SYMBOL_FUNCTION: return "function";
        case SYMBOL_PARAMETER: return "parameter";
        default: return "unknown";
    }
}